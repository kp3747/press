typedef enum
{
	line_token_type_none,
	line_token_type_eof,
	line_token_type_newline,
	line_token_type_metadata,
	line_token_type_paragraph,
	line_token_type_heading_1,
	line_token_type_heading_2,
	line_token_type_heading_3,
	line_token_type_reference,
	line_token_type_preformatted,
	line_token_type_ordered_list,
	line_token_type_unordered_list,
	line_token_type_block_nmewline,
	line_token_type_block_paragraph,
	line_token_type_block_ordered_list,
	line_token_type_block_unordered_list
} line_token_type;

typedef enum
{
	text_token_type_quote,
	text_token_type_strong,
	text_token_type_en_dash,
	text_token_type_em_dash,
	text_token_type_emphasis,
	text_token_type_reference,
	text_token_type_apostrophe,
	text_token_type_preformatted
} text_token_type;

typedef struct
{
	uint32_t offset;
	uint32_t length	: 29;
	uint32_t type	: 3;	// text_token_type
} line_token;

static_assert(sizeof(line_token) == 8);									// Prevent accidental change
static_assert((sizeof(line_token) & (sizeof(line_token) - 1)) == 0);	// Ensure power of two

typedef struct
{
	char*		buffer;
	const char*	read_ptr;
	char*		write_ptr;
	line_token*	lines;
	uint32_t	line_count;
	uint32_t	line_capacity;
	uint32_t	line;
	uint32_t	column;
	uint32_t	next_line;
	uint32_t	next_column;
	char		c;
	char		pc;
} tokenise_context;

static void handle_tokenise_error(tokenise_context* ctx, const char* format, ...)
{
	fprintf(stderr, "Tokenisation error (line %u, column %u): ", ctx->line, ctx->column);

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
		Allocate enough memory plus two bytes:
		1. Potential extra new line character before null terminator to make parsing simpler.
		2. Null terminator.
	*/
	char* data = malloc(size + 2);

	// Put data one byte past the beginning of the buffer to allow space for initial control code
	fread(data, 1, size, f);

	// Check if final new line character needs to be added, then null terminate
	if (data[size - 1] == '\n')
	{
		data[size] = 0;
	}
	else
	{
		data[size] = '\n';
		data[size + 1] = 0;
	}

	return data;
}

static uint32_t get_offset(tokenise_context* ctx)
{
	return (uint32_t)(ctx->buffer - ctx->write_ptr);
}

static line_token* add_line_token(tokenise_context* ctx, line_token_type type)
{
	line_token* line;

	if (ctx->line_count == ctx->line_capacity)
	{
		const uint32_t elements_per_page = page_size / sizeof(line_token);
		const uint32_t current_size = ctx->line_capacity ? ctx->line_capacity / elements_per_page : page_size;
		const uint32_t next_size = current_size + page_size;

		ctx->lines = realloc(ctx->lines, next_size);
		ctx->line_capacity += elements_per_page;
	}

	line = &ctx->lines[ctx->line_count++];
	line->offset = (uint32_t)(ctx->buffer - ctx->write_ptr);
	line->type = type;
}

static char get_char(tokenise_context* ctx)
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
					handle_tokenise_error(ctx, "Unsupported control character; this error may be caused by file corruption or attempting to load a binary file.");
				}
			}
			else if (c == '\n')
			{
				++ctx->next_line;
				ctx->next_column = 1;
			}
			else
			{
				handle_tokenise_error(ctx, "Unsupported control character; this error may be caused by file corruption or attempting to load a binary file.");
			}
		}
		else if (c == 127)
		{
			handle_tokenise_error(ctx, "Unsupported control character; this error may be caused by file corruption or attempting to load a binary file.");
		}
		else
		{
			++ctx->next_column;
		}
	}

	ctx->c = c;

	return c;
}

static void put_char(tokenise_context* ctx, char c)
{
	*ctx->write_ptr++ = c;
}

static void put_text_token(tokenise_context* ctx, text_token_type token)
{
	*ctx->write_ptr++ = token;
}

static bool check_space(tokenise_context* ctx, char c)
{
	if (c == ' ')
	{
		if (ctx->pc == ' ')
			handle_tokenise_error(ctx, "Extraneous space.");

		put_char(ctx, c);

		return true;
	}
	else if (c == '\t')
	{
		handle_tokenise_error(ctx, "Tabs are only permitted on a new line to represent a block quote.");
	}

	return false;
}

static bool check_emphasis(tokenise_context* ctx, char c)
{
	if (c == '*')
	{
		if (ctx->read_ptr[0] == '*')
		{
			if (ctx->read_ptr[1] == '*')
				handle_tokenise_error(ctx, "Only two levels of '*' allowed.");

			put_text_token(ctx, text_token_type_strong);
			get_char(ctx);

			if (ctx->read_ptr[0] == ' ')
				handle_tokenise_error(ctx, "Spaces are not permitted after strong (**) markup.");
		}
		else
		{
			put_text_token(ctx, text_token_type_emphasis);

			if (ctx->read_ptr[0] == ' ')
				handle_tokenise_error(ctx, "Spaces are not permitted after emphasis (*) markup.");
		}

		return true;
	}

	return false;
}

static bool check_dash(tokenise_context* ctx, char c)
{
	if (c == '-' && ctx->read_ptr[0] == '-')
	{
		if (ctx->read_ptr[1] == '-')
		{
			if (ctx->read_ptr[2] == '-')
				handle_tokenise_error(ctx, "Too many hyphens.\n");

			put_text_token(ctx, text_token_type_em_dash);
			get_char(ctx);
			get_char(ctx);
		}
		else
		{
			put_text_token(ctx, text_token_type_en_dash);
			get_char(ctx);
		}

		return true;
	}

	return false;
}

static bool check_newline(tokenise_context* ctx, char c, line_token* line)
{
	if (c == '\n')
	{
//		if (ctx->pc == ' ')
//			handle_tokenise_error(ctx, "Extraneous space.");

		const uint32_t end = get_offset(ctx);
		line->length = end - line->offset;

		return true;
	}

	return false;
}

static char tokenise_heading(tokenise_context* ctx, char c)
{
	uint32_t depth = 0;
	do
	{
		++depth;
		c = get_char(ctx);
	} while (c == '#');

	if (depth > 2)
		handle_tokenise_error(ctx, "Exceeded maximum heading depth of 3.");

	c = get_char(ctx);
	if (c != ' ')
		handle_tokenise_error(ctx, "Heading tags '#' require a following space.");

	line_token* line = add_line_token(ctx, line_token_type_heading_1 + depth);

	for (;;)
	{
		c = get_char(ctx);

		if (check_space(ctx, c))
		{
		}
		else if (check_emphasis(ctx, c))
		{
		}
		else if (check_dash(ctx, c))
		{
		}
		else if (check_newline(ctx, c, line))
		{
			return get_char(ctx);
		}
		else
		{
			put_char(ctx, c);
		}
	}
}

static void tokenise_lines(tokenise_context* ctx)
{
	ctx->line = 0;
	ctx->column = 0;
	ctx->next_line = 1;
	ctx->next_column = 1;

	char c = get_char(ctx);
	for (;;)
	{
		switch (c)
		{
		case '#':
			c = tokenise_heading(ctx, c);
			break;
		}
	}
}