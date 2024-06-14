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
	control_code_break,
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
	char		c;
	char		pc;
} parse_context;

noreturn
static void handle_parse_error(parse_context* ctx, const char* format, ...)
{
	fprintf(stderr, "Error (line %u, column %u): ", ctx->line, ctx->column);

	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);

	fputc('\n', stderr);

	exit(EXIT_FAILURE);
}

static char* load_file(const char* filepath)
{
	FILE* f = fopen(filepath, "rb");
	if (!f)
		fprintf(stderr, "Error opening file '%s': %s.\n", filepath, strerror(errno)); 

	// Get file size
	fseek(f, 0, SEEK_END);
	const long size = ftell(f);
	rewind(f);

	/*
		Allocate enough memory plus three bytes:
		1. Initial control code.
		2. Potential extra new line character before null terminator to make parsing simpler.
		3. Null terminator.
	*/
	char* data = malloc(size + 3);

	// Put data one byte past the beginning of the buffer to allow space for initial control code
	fread(data + 1, 1, size, f);

	// Check if final new line character needs to be added, then null terminate
	if (data[size] == '\n')
	{
		data[size + 1] = 0;
	}
	else
	{
		data[size + 1] = '\n';
		data[size + 2] = 0;
	}

	return data;
}

static char get_char(parse_context* ctx)
{
	ctx->pc = ctx->c;
	char c = *ctx->read_ptr++;

	ctx->line = ctx->next_line;
	ctx->column = ctx->next_column;

	if (c & 0b10000000) // UTF-8 code point with multiple bytes
	{
		if (!(c & 0b01000000))
		{
			// Ignore bytes in the middle of a UTF-8 code point
		}
		else
		{
			// First byte, so increment position
			++ctx->next_column;
		}
	}
	else // ASCII
	{
		if (c < 32)
		{
			if (c == 0)
			{
				return 0;
			}
			else if (c == '\t')
			{
			}
			else if (c == '\r')
			{
				if (*ctx->read_ptr == '\n')
				{
					c = *ctx->read_ptr++;

					++ctx->next_line;
					ctx->next_column = 1;
				}
				else
				{
					handle_parse_error(ctx, "Unsupported control character; this error may be caused by file corruption or attempting to load a binary file.");
				}
			}
			else if (c == '\n')
			{
				++ctx->next_line;
				ctx->next_column = 1;
			}
			else
			{
				handle_parse_error(ctx, "Unsupported control character; this error may be caused by file corruption or attempting to load a binary file.");
			}
		}
		else if (c == 127)
		{
			handle_parse_error(ctx, "Unsupported control character; this error may be caused by file corruption or attempting to load a binary file.");
		}
		else
		{
			++ctx->next_column;
		}
	}

	ctx-> c = c;

	return c;
}

static char consume_char(parse_context* ctx)
{
	const char c = get_char(ctx);
	assert(c);

	return c;
}

static char consume_chars(parse_context* ctx, uint32_t n)
{
	assert(n >0);

	char c;
	for (uint32_t i = 0; i < n; ++i)
	{
		c = get_char(ctx);
		assert(c);
	}

	return c;
}

static void put_char(parse_context* ctx, char c)
{
	*ctx->write_ptr++ = c;
}

static void put_control_code(parse_context* ctx, control_code code)
{
	*ctx->write_ptr++ = code;
}

static char skip_line(parse_context* ctx)
{
	char c;
	do
	{
		c = get_char(ctx);
	} while (c != '\n');

	return get_char(ctx);
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
		}
	}
	else if (c == '*')
	{
		for (;;)
		{
			c = get_char(ctx);
			if (c == '\n' && ctx->read_ptr[0] == '*' && ctx->read_ptr[1] == '/')
			{
				c = consume_chars(ctx, 3);
				if (c == '\n')
				{
					return get_char(ctx);
				}
				else
				{
					handle_parse_error(ctx, "Unexpected character after end of C-style (/*...*/) comment.");
				}
			}
			else if (c == 0)
			{
				handle_parse_error(ctx, "Unexpected end of file inside C-style (/*...*/) comment.");
			}
		}
	}
	else
	{
		handle_parse_error(ctx, "Expected '/' or '*'.");
	}
}

static bool check_emphasis(parse_context* ctx, char c)
{
	if (c == '*')
	{
		if (ctx->read_ptr[0] == '*')
		{
			if (ctx->read_ptr[1] == '*')
				handle_parse_error(ctx, "Only two levels of '*' allowed.");

			put_control_code(ctx, control_code_strong);
			consume_char(ctx);

			if (ctx->read_ptr[0] == ' ')
				handle_parse_error(ctx, "Spaces are not permitted after strong (**) markup.");
		}
		else
		{
			put_control_code(ctx, control_code_emphasis);

			if (ctx->read_ptr[0] == ' ')
				handle_parse_error(ctx, "Spaces are not permitted after emphasis (*) markup.");
		}

		return true;
	}

	return false;
}

static bool check_dash(parse_context* ctx, char c)
{
	if (c == '-' && ctx->read_ptr[0] == '-')
	{
		if (ctx->read_ptr[1] == '-')
		{
			if (ctx->read_ptr[2] == '-')
				handle_parse_error(ctx, "Too many hyphens.\n");

			put_control_code(ctx, control_code_em_dash);
			consume_chars(ctx, 2);
		}
		else
		{
			put_control_code(ctx, control_code_en_dash);
			consume_char(ctx);
		}

		return true;
	}

	return false;
}

static bool check_space(parse_context* ctx, char c)
{
	if (c == ' ')
	{
		if (ctx->pc == ' ')
			handle_parse_error(ctx, "Extraneous space.");

		put_char(ctx, c);
	}
	else if (c == '\t')
	{
		handle_parse_error(ctx, "Tabs are only permitted on a new line to represent a block quote.");
	}

	return true;
}

static bool check_newline(parse_context* ctx, char c)
{
	if (c == '\n')
	{
		if (ctx->pc == ' ')
			handle_parse_error(ctx, "Extraneous space.");

		return true;
	}

	return false;
}

static char parse_heading(parse_context* ctx, char c)
{
	uint32_t depth = 0;
	do
	{
		++depth;
		c = get_char(ctx);
	} while (c == '#');

	if (depth > 2)
		handle_parse_error(ctx, "Exceeded maximum heading depth of 3.");

	c = get_char(ctx);
	if (c != ' ')
		handle_parse_error(ctx, "Heading tags '#' require a following space.");

	put_control_code(ctx, control_code_heading_1 + depth);

	for (;;)
	{
		if (check_dash(ctx, c))
		{
		}
		else if (check_space(ctx, c))
		{
		}
		else if (check_newline(ctx, c))
		{
			break;
		}
		else
		{
			put_char(ctx, c);
		}

		c = get_char(ctx);
	}

	c = get_char(ctx);
	if (c != '\n')
		handle_parse_error(ctx, "Headings must be followed by an empty line.");

	return get_char(ctx);
}

static char parse_paragraph(parse_context* ctx, char c)
{
	put_control_code(ctx, control_code_paragraph);

	for (;;)
	{
		if (check_emphasis(ctx, c))
		{
		}
		else if (check_dash(ctx, c))
		{
		}
		else if (check_space(ctx, c))
		{
		}
		else if (check_newline(ctx, c))
		{
			c = get_char(ctx);
			if (c == '\n')
				return get_char(ctx);
			else
				put_control_code(ctx, control_code_break);
		}
		else
		{
			put_char(ctx, c);
		}

		c = get_char(ctx);
	}
}

static void parse(parse_context* ctx)
{
	ctx->line = 1;
	ctx->column = 1;
	ctx->next_line = 1;
	ctx->next_column = 1;

	char c = get_char(ctx);
	for (;;)
	{
		switch (c)
		{
		case '#':
			c = parse_heading(ctx, c);
			break;
		case '\n':
			handle_parse_error(ctx, "Unnecessary blank line.");
		case '/':
			consume_char(ctx);
			c = skip_comment(ctx);
			break;
		case '[':
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
			c = skip_line(ctx);
			break;
		case 0:
			return;
		default:
			c = parse_paragraph(ctx, c);
		}
	}
}

int main(void)
{
	char* text = load_file("C:\\dev\\press\\doc\\Combat Liberalism - Mao Zedong.txt");

	return EXIT_SUCCESS;
}