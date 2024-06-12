/*
	NOTES:
	* Add the concept of books as well as chapters. e.g. The bible or a collection of books.
	* https://devblogs.microsoft.com/commandline/tar-and-curl-come-to-windows/
*/

#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <setjmp.h>
#include <assert.h>
#include <stdbool.h>

#if __STDC_VERSION__ < 202311		// <C23
	#define nullptr ((void*)0)
#endif

#if __STDC_VERSION__ >= 202311		// >=C23
	#define noreturn [[noreturn]]
#elif __STDC_VERSION__ >= 201112	// >=C11
	#define noreturn _Noreturn		// Deprecated in C23
#else
	#define noreturn
#endif

typedef enum
{
	control_code_eof,
	control_code_strong,
	control_code_en_dash,	// En and em hashes are control code because UTF-8 requires 3 bytes,
	control_code_em_dash,	// while an en hash in source text uses only 2 bytes ("--").
	control_code_emphasis,
	control_code_heading_1,
	control_code_heading_2,
	control_code_heading_3,
	control_code_heading_4,
	control_code_heading_5,
	control_code_heading_6,
	control_code_paragraph,
	control_code_reference,
	control_code_blockquote,
	control_code_ordered_list,
	control_code_unordered_list
} control_code;

typedef struct
{
	char*		buffer;
	const char*	read_ptr;
	char*		write_ptr;
	uint32_t	line;
	uint32_t	column;
	uint32_t	next_line;
	uint32_t	next_column;
} parse_context;

noreturn
static void handle_error(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);

	exit(EXIT_FAILURE);
}

static char* load_file(const char* filepath)
{
	FILE* f = fopen(filepath, "rb");
	if (!f)
		handle_error("Error opening file '%s': %s.\n", filepath, strerror(errno));

	// Get file size
	fseek(f, 0, SEEK_END);
	const long size = ftell(f);
	rewind(f);

	// Allocate memory and null terminate
	char* data = malloc(size + 1);
	data[size] = 0;

	fread(data, 1, size, f);

	return data;
}

int64_t get_utf8_byte_count(char c)
{
	if ((c & 0b11110000) == 0b11110000)
		return 4;
	else if ((c & 0b11100000) == 0b11100000)
		return 3;
	else if ((c & 0b11000000) == 0b11000000)
		return 2;
	else
		return 1;
}

static char get_char(parse_context* ctx)
{
	const char c = *ctx->read_ptr++;

	ctx->next_line = ctx->line;
	ctx->next_column = ctx->column;

	if (c < 128)
	{
		if (c < 32)
		{
		}
		else if (c == 127)
		{
			handle_error("Error: Control characters are not permitted; this error may be caused by file corruption or attempting to load a binary file.\n");
		}
	}

	return 0;
}

//static char peek_char(parse_context* ctx)
//{
//	return *ctx->read_ptr;
//}

static void consume_char(parse_context* ctx)
{
	++ctx->read_ptr;
}

// TODO: Check for going past EOF
static void consume_chars(parse_context* ctx, uint32_t n)
{
	ctx->read_ptr += n;
}

static void put_char(parse_context* ctx, char c)
{
	*ctx->write_ptr++ = c;
}

static void put_control_code(parse_context* ctx, control_code code)
{
	*ctx->write_ptr++ = code;
}

static char skip_comment(parse_context* ctx)
{
	char c = get_char(ctx);
	if (c == '/')
	{
		// Skip past the end of the line for C++ comments
		for (;;)
		{
			c = get_char(ctx);
			if (c == '\n')
				return get_char(ctx);
			else if (c == 0)
				return  0;
		}
	}
	else if (c == '*')
	{
		for (;;)
		{
			c = get_char(ctx);
			if (c == '\n' && ctx->read_ptr[0] == '*' && ctx->read_ptr[1] == '/')
			{
				if (ctx->read_ptr[2] == '\n')
				{
					consume_chars(ctx, 3);
					return get_char(ctx);
				}
				else if (ctx->read_ptr[2] == '\r' && ctx->read_ptr[3] == '\n')
				{
					consume_chars(ctx, 4);
					return get_char(ctx);
				}
				else if (ctx->read_ptr[2] == 0)
				{
					return 0;
				}
				else
				{
					handle_error("Error: Unexpected character after end of C-style (/*...*/) comment.\n");
				}
			}
			else if (c == 0)
			{
				handle_error("Error: Unexpected end of file inside C-style (/*...*/) comment.\n");
			}
		}
	}
	else
	{
		handle_error("Error: Expected '/' or '*'.\n");
	}
}

static char parse_paragraph(parse_context* ctx)
{
	put_control_code(ctx, control_code_paragraph);

	char c = 0;
	char prev;

	for (;;)
	{
		prev = c;
		c = get_char(ctx);

		if (c == '*')
		{
			if (ctx->read_ptr[0] == '*')
			{
				if (ctx->read_ptr[1] == '*')
					handle_error("Error: Only two levels of '*' allowed.\n");

				put_control_code(ctx, control_code_strong);
				consume_char(ctx);

				if (ctx->read_ptr[0] == ' ')
					handle_error("Error: Spaces are not permitted after strong (**) markup.\n");
			}
			else
			{
				put_control_code(ctx, control_code_emphasis);

				if (ctx->read_ptr[0] == ' ')
					handle_error("Error: Spaces are not permitted after emphasis (*) markup.\n");
			}
		}
		else if (c == '-' && ctx->read_ptr[0] == '-')
		{
			put_control_code(ctx, control_code_en_dash);
			consume_char(ctx);
		}
		else if (c == '-' && ctx->read_ptr[0] == '-' && ctx->read_ptr[1] == '-')
		{
			put_control_code(ctx, control_code_em_dash);
			consume_chars(ctx, 2);
		}
		else if (c == '\r' && ctx->read_ptr[0] == '\n')
		{
			if (prev == ' ')
				handle_error("Error: Extraneous space.\n");

			consume_char(ctx);
			return get_char(ctx);
		}
		else if (c == '\n')
		{
			if (prev == ' ')
				handle_error("Error: Extraneous space.\n");

			return get_char(ctx);
		}
		else if (c == ' ')
		{
			if (prev == ' ')
				handle_error("Error: Extraneous space.\n");
		}
		else if (c == '\t')
		{
			handle_error("Error: Tabs are only permitted on a new line to represent a block quote.\n");
		}
		else
		{
			put_char(ctx, c);
		}
	}
}

static void parse(parse_context* ctx)
{
	//ctx->lines = nullptr;
	//ctx->line_count = 0;

	char c = *ctx->read_ptr;

	for (;;)
	{
		//ctx->prev_char = 0;

		switch (c)
		{
		case '\n':
			handle_error("Error: Unnecessary blank line.");
		case '/':
			consume_char(ctx);
			c = skip_comment(ctx);
			break;
		case '[':
		case '#':
		case '\t':
		case '*':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'I':
		case 'V':
		case 'X':
		case 0:
			return;
		default:
			c = parse_paragraph(ctx);
		}
	}
}

//static char get_char(parse_context* ctx)
//{
//	char c = *ctx->read_ptr++;
//
//	// Ignore \r to handle both Windows and Unix line endings
//	if (c == '\r')
//		c = *ctx->ptr++;
//
//	if (c == 0)
//		longjmp(ctx->jmp_ctx, 1);
//
//	return c;
//}
//
//static char peek_char(parse_context* ctx)
//{
//	char c = *ctx->ptr;
//
//	// Ignore \r to handle both Windows and Unix line endings
//	if (c == '\r')
//		c = *ctx->ptr++;
//
//	if (c == 0)
//		longjmp(ctx->jmp_ctx, 1);
//
//	return c;
//}
//
//static void consume_char(parse_context* ctx)
//{
//	assert(*ctx->ptr);
//
//	++ctx->ptr;
//}
//
//static void parse(parse_context* ctx)
//{
//	//ctx->lines = nullptr;
//	//ctx->line_count = 0;
//
//	for (;;)
//	{
//		//ctx->prev_char = 0;
//
//		switch (*ctx->text)
//		{
//		case '\r':
//			// Skip past extra character used in Windows line endings
//			++ctx->ptr;
//			break;
//		case '\n':
//			handle_error("Error: Unnecessary blank line.");
//		case '/':
//			skip_comment(ctx);
//			break;
//		case '[':
//		case '#':
//		case '\t':
//		case '*':
//		case '1':
//		case '2':
//		case '3':
//		case '4':
//		case '5':
//		case '6':
//		case '7':
//		case '8':
//		case '9':
//		default:
//			parse_paragraph(ctx);
//		}
//	}
//}

//typedef enum
//{
//	document_type_book,
//	document_type_article,
//	document_type_pamphlet
//} document_type;
//
//typedef enum
//{
//	document_element_type_heading,
//	document_element_type_paragraph,
//	document_element_type_blockquote
//} document_element_type;
//
//typedef struct
//{
//	uint16_t	year;
//	uint8_t		month;
//	uint8_t		day;
//} document_date;
//
//typedef struct
//{
//	document_element_type	type;
//	uint32_t				level;
//	const char*				text;
//} document_element;
//
//typedef struct
//{
//	const char*			title;
//	const char*			text;
//	bool				roman;
//	uint32_t			number;
//	document_element*	elements;
//	uint32_t			element_count;
//} document_chapter;
//
//typedef struct
//{
//	document_type		type;
//	const char*			title;
//	const char**		authors;
//	document_date		written;
//	document_date		published;
//	document_chapter*	chapters;
//	uint32_t			author_count;
//	uint32_t			chapter_count;
//} document;
//
///*
//	Because of the nature of the source format, we don't need to or want to fully tokenise the text.
//	This isn't a programming language which ignores whitespace and used many symbols. The format is
//	primarily line-based as the first character of each line dictates the type. This simplifies the
//	parsing algorithm.
//*/
//typedef enum
//{
//	line_token_type_empty,
//	line_token_type_chapter,
//	line_token_type_heading,
//	line_token_type_metadata,
//	line_token_type_footnote,
//	line_token_type_paragraph,
//	line_token_type_blockquote,
//	line_token_type_ordered_list,
//	line_token_type_unordered_list
//} line_token_type;
//
//typedef struct
//{
//	line_token_type	type;
//	char*			begin;
//	char*			end;
//} line_token;
//
//typedef struct
//{
//	char*		ptr;
//	uint32_t	size;
//	line_token*	lines;
//	uint32_t	line_count;
//	char		prev_char;
//	jmp_buf		jmp_ctx;
//} parse_context;
//
//static void skip_comment(parse_context* ctx)
//{
//}
//
//static void skip_line(parse_context* ctx)
//{
//}
//
//noreturn
//static void parse_finalise(parse_context* ctx)
//{
//}
//
//static bool check_newline(parse_context* ctx)
//{
//	if (ctx->ptr[0] == '\r')
//	{
//		if (ctx->ptr[1] != '\n')
//			handle_error("Error: Carriage return cannot be used without subsequent new line character.\n");
//
//		ctx->ptr++;
//
//		return true;
//	}
//	else if (ctx->ptr[0] == '\n')
//	{
//		ctx->ptr++;
//
//		return true;
//	}
//
//	return false;
//}
//
//static void check_ctrl_char(parse_context* ctx)
//{
//	if (*ctx->ptr != '\t')
//		handle_error("Error: Tab characters are only allowed to represent block quotes at the beginning of a line.\n");
//	else if (*ctx->ptr < ' ' || *ctx->ptr == 127)
//		handle_error("Error: Control characters are not permitted; this error may be caused by file corruption or attempting to load a binary file.\n");
//}
//
//static void parse_paragraph(parse_context* ctx)
//{
//	if (*ctx->ptr == ' ')
//		handle_error("Error: Leading spaces are not permitted.\n");
//
//	for (;;)
//	{
//		if (*ctx->ptr == ' ')
//		{
//			if (ctx->prev_char == ' ')
//				handle_error("Error: unnecessary spaces are not permitted.\n");
//		}
//		else if (check_newline(ctx))	// New lines are allowed within paragraphs
//		{
//			if (check_newline(ctx))		// But an empty line signifies a new element
//				return;
//		}
//		else if (*ctx->ptr == 0)
//		{
//			parse_finalise(ctx);
//		}
//		else
//		{
//			check_ctrl_char(ctx);
//		}
//
//		ctx->prev_char = *ctx->ptr;
//		++ctx->ptr;
//	}
//}
//
//// TODO: Track line and character numbers for error reporting
//static void validate_element(const char* begin, const char* end)
//{
//	const int64_t len = end - begin;
//	if (len == 0)
//		handle_error("Error: Empty element.\n");
//
//	// TODO: Ensure no tabs or duplicate spaces
//}
//
//static bool calculate_heading_size(parse_context* ctx)
//{
//	char c = peek_char(ctx);
//	if (c == '#')
//	{
//		consume_char(ctx);
//
//		// Calculate header depth
//		uint32_t header_depth = 1;
//		//while (
//
//		ctx->size += sizeof(document_element);
//	}
//
//	return false;
//}
//
//static bool calculate_paragraph_size(parse_context* ctx)
//{
//	char c = peek_char(ctx);
//	if (c == '\n')
//	{
//		consume_char(ctx);
//
//		char* begin = ctx->ptr;
//
//		ctx->size += sizeof(document_element);
//
//		for (;;)
//		{
//			c = get_char(ctx);
//			if (c == '\n')
//			{
//				//validate_text(begin, ctx->ptr);
//
//				return true;
//			}
//		}
//	}
//
//	return false;
//}
//
//static char* parse_blockquote(char* text)
//{
//	for (;;)
//	{
//		if (*text == '\n' && text[1] != '/t')
//			return text;
//		else if (*text == 0)
//			return text;
//
//		++text;
//	}
//}

//static document* parse_document(char* text)
//{
//	parse_context ctx;
//	ctx.ptr = text;
//	ctx.size = sizeof(document);
//
//	if (!setjmp(ctx.jmp_ctx))
//	{
//	}
//
////	uint32_t size = sizeof(document);
////	bool newline = true;
////	char* current = text;
////
////	// First calculate amount of memory required
////	for (;;)
////	{
////		if (*current == 0)
////		{
////			break;
////		}
////		else if (newline)
////		{
////			if (*current == '\t')
////			{
////				size += sizeof(document_element);
////				current = parse_blockquote(current);
////			}
////			else
////			{
////				uint32_t header_depth = 0;
////				while (*current == '#')
////					++header_depth;
////
////				if (header_depth > 0)
////				{
////					if (header_depth == 1)
////						size += sizeof(document_chapter);
////					else
////						size += sizeof(document_element);
////				}
////				else
////				{
////					size += sizeof(document_element);
////					current = parse_paragraph(current);
////				}
////			}
////		}
////		else if (*current == '\n')
////		{
////			newline = false;
////		}
////		else if (*current == '\r')
////		{
////			// Ignore Windows line endings
////		}
////
////		++current;
////	}
//
//	return nullptr;
//}

int main(void)
{
	char* text = load_file("C:\\dev\\press\\doc\\Combat Liberalism - Mao Zedong.txt");
	//document* doc = parse_document(text);

	return EXIT_SUCCESS;
}