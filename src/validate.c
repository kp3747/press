typedef struct
{
	line_token*	tokens;
	uint32_t	line;
	uint32_t	current;
	uint32_t	token_count;
	uint32_t	author_count;
	uint32_t	chapter_count;
	uint32_t	element_count;
	uint32_t	reference_count;
	uint32_t	translator_count;
} validate_context;

static void handle_validate_error(validate_context* ctx, const char* format, ...)
{
	fprintf(stderr, "Parsing error (line %u): ", ctx->line);

	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);

	fputc('\n', stderr);

	exit(EXIT_FAILURE);
}

static line_token* validate_get_next_token(validate_context* ctx)
{
	assert(ctx->current < ctx->token_count);

	line_token* token = &ctx->tokens[ctx->current++];
	ctx->line = token->line;

	return token;
}

static line_token* validate_newline(validate_context* ctx, line_token* token)
{
	line_token* next = validate_get_next_token(ctx);
	if (next->type == line_token_type_newline)
		handle_validate_error(ctx, "Extra blank lines are not permitted.");

	return next;
}

// TODO: Parse metadata
static line_token* validate_metadata(validate_context* ctx, line_token* token)
{
	return validate_get_next_token(ctx);
}

static line_token* validate_paragraph(validate_context* ctx, line_token* token)
{
	++ctx->token_count;

	line_token* next = validate_get_next_token(ctx);
	do
	{
		next = validate_get_next_token(ctx);
	} while (next->type == line_token_type_paragraph);

	if (next->type != line_token_type_newline)
		handle_validate_error(ctx, "Paragraphs must be followed by a blank line.");

	return next;
}

static line_token* validate_heading(validate_context* ctx, line_token* token)
{
	const int level = token->type - line_token_type_heading_1;
	assert(level >= 0);
	assert(level < 3);

	if (level == 0)
		++ctx->chapter_count;
	else
		++ctx->token_count;

	line_token* next = validate_get_next_token(ctx);
	if (next->type != line_token_type_newline)
		handle_validate_error(ctx, "Headings must be followed by a blank line.");

	return next;
}

static line_token* validate_reference(validate_context* ctx, line_token* token)
{
	++ctx->reference_count;

	line_token* next = validate_get_next_token(ctx);
	if (next->type != line_token_type_newline)
		handle_validate_error(ctx, "References must be followed by a blank line.");

	return next;
}

static line_token* validate_preformatted(validate_context* ctx, line_token* token)
{
	++ctx->token_count;

	line_token* next = validate_get_next_token(ctx);
	if (next->type != line_token_type_newline)
		handle_validate_error(ctx, "Preformatted blocks must be followed by a blank line.");

	return next;
}

static line_token* validate_block_newline(validate_context* ctx, line_token* token)
{
	token = validate_get_next_token(ctx);
	if (token->type != line_token_type_block_paragraph)
		handle_validate_error(ctx, "Blank lines within block quotes must be followed by an indented paragraph.");

	return token;
}

static line_token* validate_block_paragraph(validate_context* ctx, line_token* token)
{
	token = validate_get_next_token(ctx);
	if (token->type != line_token_type_block_paragraph || token->type != line_token_type_block_newline)
		handle_validate_error(ctx, "Block quotes must be followed by a blank indented line.");

	return token;
}

static line_token* validate_blockquote(validate_context* ctx, line_token* token)
{
	++ctx->token_count;

	token = validate_get_next_token(ctx);
	for (;;)
	{
		if (token->type == line_token_type_block_newline)
			token = validate_block_newline(ctx, token);
		else if (token->type == line_token_type_block_paragraph)
			token = validate_block_paragraph(ctx, token);
		else if (token->type == line_token_type_newline)
			break;
		else
			assert(false);
	}

	return token;
}

static void validate(line_tokens* tokens, doc_mem_req* out_mem_req)
{
	validate_context ctx = {
		.tokens			= tokens->lines,
		.token_count	= tokens->count
	};

	line_token* token = validate_get_next_token(&ctx);
	for (;;)
	{
		switch (token->type)
		{
		case line_token_type_eof:
			out_mem_req->author_count = ctx.author_count;
			out_mem_req->chapter_count = ctx.chapter_count;
			out_mem_req->element_count = ctx.element_count;
			out_mem_req->reference_count = ctx.reference_count;
			out_mem_req->translator_count = ctx.translator_count;
			return;
		case line_token_type_newline:
			token = validate_newline(&ctx, token);
			break;
		case line_token_type_metadata:
			token = validate_metadata(&ctx, token);
			break;
		case line_token_type_paragraph:
			token = validate_paragraph(&ctx, token);
			break;
		case line_token_type_heading_1:
		case line_token_type_heading_2:
		case line_token_type_heading_3:
			token = validate_heading(&ctx, token);
			break;
		case line_token_type_reference:
			token = validate_reference(&ctx, token);
			break;
		case line_token_type_preformatted:
			token = validate_preformatted(&ctx, token);
			break;
		case line_token_type_block_newline:
			handle_validate_error(&ctx, "Block quotes may not begin with a blank line.");
			break;
		case line_token_type_block_paragraph:
			token = validate_blockquote(&ctx, token);
			break;
		default:
			assert(false);
		}
	}
}