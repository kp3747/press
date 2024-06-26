typedef struct
{
	char*			buffer;
	const char*		read_ptr;
	char*			write_ptr;
	line_token*		current_line;
	line_token*		lines;
	uint32_t		line_count;
	uint32_t		line_capacity;
	uint32_t		line;
	uint32_t		column;
	uint32_t		next_line;
	uint32_t		next_column;
	char			c;
	char			pc;
	//line_token_type	previous_line_type;
} tokenise_context;

static uint32_t arabic_to_int(const char* str, char terminator)
{
	uint32_t digits[10];
	int current_digit = 0;

	while (*str != terminator)
		digits[current_digit++] = *str++ - '0';

	uint32_t result = 0;
	uint32_t multiplier = 1;

	for (int i = current_digit - 1; i >= 0; --i)
	{
		result += digits[i] * multiplier;
		multiplier *= 10;
	}

	return result;
}

static void handle_tokenise_error(tokenise_context* ctx, const char* format, ...)
{
	fprintf(stderr, "Parsing error (line %u, column %u): ", ctx->line, ctx->column);

	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);

	fputc('\n', stderr);

	assert(false);
	exit(EXIT_FAILURE);
}

//static uint32_t get_offset(tokenise_context* ctx)
//{
//	return (uint32_t)(ctx->buffer - ctx->write_ptr);
//}

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
	line->type = type;
	line->line = ctx->line;
	line->text = ctx->write_ptr;
	//line->offset = (uint32_t)(ctx->buffer - ctx->write_ptr);

#ifndef NDEBUG
	if (type == line_token_type_newline || type == line_token_type_eof)
		line->text = nullptr;
#endif

	ctx->current_line = line;

	return line;
}

static void set_read_ptr(tokenise_context* ctx, const char* ptr)
{
	ctx->column += (uint32_t)(ptr - ctx->read_ptr);
	ctx->read_ptr = ptr;
	ctx->c = ptr[-1];
	ctx->pc = ptr[-2];
}

static char get_char(tokenise_context* ctx)
{
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

	return c;
}

static char get_filtered_char(tokenise_context* ctx)
{
	ctx->pc = ctx->c;

	char c = get_char(ctx);
	if (c == '/' && *ctx->read_ptr == '*')
	{
		// Consume '*'
		c = get_char(ctx);

		for (;;)
		{
			if (c == 0)
			{
				handle_tokenise_error(ctx, "Comments must be closed \"/*...*/\".");
			}
			else if (c == '*' && *ctx->read_ptr == '/')
			{
				// Consume '/'
				get_char(ctx);
				c = get_char(ctx);
				break;
			}

			c = get_char(ctx);
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
			get_filtered_char(ctx);
		}
		else
		{
			put_text_token(ctx, text_token_type_emphasis);
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
				handle_tokenise_error(ctx, "Too many hyphens.");

			put_text_token(ctx, text_token_type_em_dash);
			get_filtered_char(ctx);
			get_filtered_char(ctx);
		}
		else
		{
			put_text_token(ctx, text_token_type_en_dash);
			get_filtered_char(ctx);
		}

		return true;
	}

	return false;
}

static bool check_newline(tokenise_context* ctx, char c)
{
	if (c == '\n')
	{
		if (ctx->read_ptr[0] == ' ')
			handle_tokenise_error(ctx, "Trailing spaces are not permitted.");

		//const uint32_t end = get_offset(ctx);
		ctx->current_line->length = (uint32_t)(ctx->write_ptr - ctx->current_line->text);

		// Null terminate
		put_char(ctx, 0);

		return true;
	}

	return false;
}

// TODO: Add support for quotes, references and preformatted text
static char tokenise_text(tokenise_context* ctx, char c)
{
	if (c == ' ')
		handle_tokenise_error(ctx, "Leading spaces are not permitted.");

	for (;;)
	{
		if (check_space(ctx, c))
		{
		}
		else if (check_emphasis(ctx, c))
		{
		}
		else if (check_dash(ctx, c))
		{
		}
		else if (check_newline(ctx, c))
		{
			return get_filtered_char(ctx);
		}
		else
		{
			put_char(ctx, c);
		}

		c = get_filtered_char(ctx);
	}
}

static char tokenise_newline(tokenise_context* ctx, char c, bool blockquote)
{
	const line_token_type type = blockquote ? line_token_type_block_newline : line_token_type_newline;
	add_line_token(ctx, type);

	c = get_filtered_char(ctx);

	return c;
}

static char tokenise_paragraph(tokenise_context* ctx, char c, bool blockquote)
{
	const line_token_type type = blockquote ? line_token_type_block_paragraph : line_token_type_paragraph;
	add_line_token(ctx, type);

	return tokenise_text(ctx, c);
}

static char tokenise_heading(tokenise_context* ctx, char c)
{
	int32_t depth = -1;
	do
	{
		++depth;
		c = get_filtered_char(ctx);
	} while (c == '#');

	if (depth > 2)
		handle_tokenise_error(ctx, "Exceeded maximum heading depth of 3.");

	if (c != ' ')
		handle_tokenise_error(ctx, "Heading tags '#' must be followed by a space.");

	add_line_token(ctx, line_token_type_heading_1 + depth);

	// Consume space
	c = get_filtered_char(ctx);

	return tokenise_text(ctx, c);
}

static char tokenise_ordered_list_arabic(tokenise_context* ctx, char c)
{
	const char* current = ctx->read_ptr;

	while (*current >= '0' && *current <= '9')
		++current;

	if (*current++ == '.' && *current++ == ' ')
	{
		line_token* line = add_line_token(ctx, line_token_type_ordered_list_arabic);
		line->index = arabic_to_int(ctx->read_ptr - 1, '.');

		set_read_ptr(ctx, current);

		return tokenise_text(ctx, get_filtered_char(ctx));
	}

	return tokenise_paragraph(ctx, c, false);
}

static char tokenise_ordered_list_roman_upper(tokenise_context* ctx, char c)
{
	const char* current = ctx->read_ptr;

	for (;;)
	{
		if (*current != 'I' && *current != 'V' && *current != 'X')
			break;

		++current;
	}

	if (*current == '.' && *current++ == ' ')
	{
		line_token* line = add_line_token(ctx, line_token_type_ordered_list_roman);

		for (int i = 0; i < roman_numeral_max; ++i)
		{
			if (strcmp(ctx->read_ptr - 1, roman_upper_strings[i]) == 0)
			{
				line->index = i + 1;
				set_read_ptr(ctx, current);

				return tokenise_text(ctx, get_filtered_char(ctx));
			}
		}

		handle_tokenise_error(ctx, "Ordered lists using Roman numerals currently have a maximum value of %d.", roman_numeral_max);
	}

	return tokenise_paragraph(ctx, c, false);
}

static char tokenise_ordered_list_roman_lower(tokenise_context* ctx, char c)
{
	const char* current = ctx->read_ptr;

	for (;;)
	{
		if (*current != 'i' && *current != 'v' && *current != 'x')
			break;

		++current;
	}

	if (*current == '.' && *current++ == ' ')
	{
		line_token* line = add_line_token(ctx, line_token_type_ordered_list_roman);

		for (int i = 0; i < roman_numeral_max; ++i)
		{
			if (strcmp(ctx->read_ptr - 1, roman_lower_strings[i]) == 0)
			{
				line->index = i + 1;
				set_read_ptr(ctx, current);

				return tokenise_text(ctx, get_filtered_char(ctx));
			}
		}

		handle_tokenise_error(ctx, "Ordered lists using Roman numerals currently have a maximum value of %d.", roman_numeral_max);
	}

	return tokenise_paragraph(ctx, c, false);
}

static char tokenise_ordered_list_letter(tokenise_context* ctx, char c, bool blockquote)
{
	if (ctx->read_ptr[0] == '.' && ctx->read_ptr[1] == ' ')
	{
		line_token* line = add_line_token(ctx, line_token_type_ordered_list_letter);
		line->index = c - 'a';

		set_read_ptr(ctx, ctx->read_ptr + 2);

		return tokenise_text(ctx, get_filtered_char(ctx));
	}

	return tokenise_paragraph(ctx, c, blockquote);
}

static char tokenise_unordered_list(tokenise_context* ctx, char c)
{
	c = get_filtered_char(ctx);
	if (c != ' ')
		handle_tokenise_error(ctx, "Unordered list tags '*' must be followed by a space.");

	add_line_token(ctx, line_token_type_unordered_list);

	return tokenise_text(ctx, get_filtered_char(ctx));
}

static char tokenise_blockquote(tokenise_context* ctx, char c)
{
	c = get_filtered_char(ctx);
	switch (c)
	{
	case '\n':
		c = tokenise_newline(ctx, c, true);
		break;
	default:
		c = tokenise_paragraph(ctx, c, true);
	}

	return c;
}

static char tokenise_metadata(tokenise_context* ctx, char c)
{
	// Just skip for now
	do
	{
		c = get_filtered_char(ctx);
	} while (c != '\n');

	// Consume '\n'
	c = get_filtered_char(ctx);

	return c;
}

static void tokenise(char* data, line_tokens* out_tokens)
{
	tokenise_context ctx = {
		.buffer			= data,
		.read_ptr		= data,
		.write_ptr		= data,
		.next_line		= 1,
		.next_column	= 1
	};

	char c = get_filtered_char(&ctx);
	for (;;)
	{
		switch (c)
		{
		case 0:
			out_tokens->lines = ctx.lines;
			out_tokens->count = ctx.line_count;
			return;
		case '[':
			c = tokenise_metadata(&ctx, c);
			break;
		case '\t':
			c = tokenise_blockquote(&ctx, c);
			break;
		case '\n':
			c = tokenise_newline(&ctx, c, false);
			break;
		case '#':
			c = tokenise_heading(&ctx, c);
			break;
		case '*':
			c = tokenise_unordered_list(&ctx, c);
			break;
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			c = tokenise_ordered_list_arabic(&ctx, c);
			break;
		case 'I':
		case 'V':
		case 'X':
			c = tokenise_ordered_list_roman_upper(&ctx, c);
			break;
		case 'i':
		case 'v':
		case 'x':
			c = tokenise_ordered_list_roman_lower(&ctx, c);
			break;
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
			c = tokenise_ordered_list_letter(&ctx, c, false);
			break;
		default:
			c = tokenise_paragraph(&ctx, c, false);
		}
	}
}