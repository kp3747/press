typedef struct
{
	document*			doc;
	line_token*			tokens;
	document_element*	elements;
	document_reference*	references;
	document_chapter*	current_chapter;
	uint32_t			current_element;
	uint32_t			current_reference;
	uint32_t			current_token;
	uint32_t			token_count;
	uint32_t			author_count;
	uint32_t			chapter_count;
	uint32_t			element_count;
	uint32_t			reference_count;
	uint32_t			translator_count;
} finalise_context;

static line_token* finalise_get_next_token(finalise_context* ctx)
{
	assert(ctx->current_token < ctx->token_count);

	return &ctx->tokens[ctx->current_token++];
}

static void finalise_add_element(finalise_context* ctx, document_element_type type, const char* text)
{
	assert(ctx->current_element < ctx->element_count);

	const uint32_t element_index = ctx->current_chapter->element_count++;
	++ctx->current_element;

	document_element* element = &ctx->current_chapter->elements[element_index];
	element->type = type;
	element->text = text;
}

static line_token* finalise_heading_1(finalise_context* ctx, line_token* token)
{
	const uint32_t chapter_index = ctx->doc->chapter_count++;

	document_chapter* chapter = &ctx->doc->chapters[chapter_index];
	chapter->elements = &ctx->elements[ctx->current_element];
	chapter->references = &ctx->references[ctx->current_reference];
	chapter->element_count = 0;
	chapter->reference_count = 0;

	ctx->current_chapter = chapter;

	finalise_add_element(ctx, document_element_type_heading_1, token->text);

	return finalise_get_next_token(ctx);
}

static line_token* finalise_heading_2(finalise_context* ctx, line_token* token)
{
	finalise_add_element(ctx, document_element_type_heading_2, token->text);

	return finalise_get_next_token(ctx);
}

static line_token* finalise_heading_3(finalise_context* ctx, line_token* token)
{
	finalise_add_element(ctx, document_element_type_heading_3, token->text);

	return finalise_get_next_token(ctx);
}

static line_token* finalise_paragraph(finalise_context* ctx, line_token* token)
{
	finalise_add_element(ctx, document_element_type_paragraph_begin, nullptr);
	finalise_add_element(ctx, document_element_type_text_block, token->text);

	token = finalise_get_next_token(ctx);
	while (token->type == line_token_type_paragraph)
	{
		finalise_add_element(ctx, document_element_type_line_break, nullptr);
		finalise_add_element(ctx, document_element_type_text_block, token->text);
		token = finalise_get_next_token(ctx);
	}

	finalise_add_element(ctx, document_element_type_paragraph_end, nullptr);

	return token;
}

static line_token* finalise_blockquote(finalise_context* ctx, line_token* token)
{
	finalise_add_element(ctx, document_element_type_blockquote_begin, nullptr);

	for (;;)
	{
		if (token->type == line_token_type_block_newline)
		{
			token = finalise_get_next_token(ctx);
		}
		else if (token->type == line_token_type_block_paragraph)
		{
			finalise_add_element(ctx, document_element_type_paragraph_begin, nullptr);
			finalise_add_element(ctx, document_element_type_text_block, token->text);

			token = finalise_get_next_token(ctx);
			while (token->type == line_token_type_block_paragraph)
			{
				finalise_add_element(ctx, document_element_type_line_break, nullptr);
				finalise_add_element(ctx, document_element_type_text_block, token->text);
				token = finalise_get_next_token(ctx);
			}

			finalise_add_element(ctx, document_element_type_paragraph_end, nullptr);
		}
		else
		{
			break;
		}
	}

	finalise_add_element(ctx, document_element_type_blockquote_end, nullptr);

	return token;
}

static line_token* finalise_ordered_list_roman(finalise_context* ctx, line_token* token)
{
	finalise_add_element(ctx, document_element_type_ordered_list_begin_roman, nullptr);
	finalise_add_element(ctx, document_element_type_ordered_list_item, token->text);

	token = finalise_get_next_token(ctx);
	while (token->type == line_token_type_ordered_list_roman)
	{
		finalise_add_element(ctx, document_element_type_ordered_list_item, token->text);
		token = finalise_get_next_token(ctx);
	}

	finalise_add_element(ctx, document_element_type_ordered_list_end, nullptr);

	return token;
}

static void finalise(line_tokens* tokens, const doc_mem_req* mem_req, document* out_doc)
{
	const size_t author_size = sizeof(const char*) * mem_req->author_count;
	const size_t translator_size = sizeof(const char*) * mem_req->translator_count;
	const size_t chapter_size = sizeof(document_chapter) * mem_req->chapter_count;
	const size_t element_size = sizeof(document_element) * mem_req->element_count;
	const size_t reference_size = sizeof(document_reference) * mem_req->reference_count;

	const size_t total_size =
		author_size +
		translator_size +
		chapter_size +
		element_size +
		reference_size
	;
	uint8_t* mem = malloc(total_size);

	memset(out_doc, 0x00, sizeof(document));

	out_doc->metadata.authors = (const char**)mem;
	mem += author_size;

	out_doc->metadata.translators = (const char**)mem;
	mem += translator_size;

	out_doc->chapters = (document_chapter*)mem;
	mem += chapter_size;

	document_element* elements = (document_element*)mem;
	mem += element_size;

	document_reference* references = (document_reference*)mem;
	mem += reference_size;

	out_doc->metadata.author_count = mem_req->author_count;
	out_doc->metadata.translator_count = mem_req->translator_count;

	finalise_context ctx = {
		.doc				= out_doc,
		.tokens				= tokens->lines,
		.elements			= elements,
		.references			= references,
		.token_count		= tokens->count,
		.author_count		= mem_req->author_count,
		.chapter_count		= mem_req->chapter_count,
		.element_count		= mem_req->element_count,
		.reference_count	= mem_req->reference_count,
		.translator_count	= mem_req->translator_count
	};

	line_token* token = finalise_get_next_token(&ctx);
	for (;;)
	{
		switch (token->type)
		{
		case line_token_type_eof:
			assert(ctx.current_element == ctx.element_count);
			return;
		case line_token_type_metadata:
			//token = finalise_metadata(&ctx, token);
			break;
		case line_token_type_paragraph:
			token = finalise_paragraph(&ctx, token);
			break;
		case line_token_type_heading_1:
			token = finalise_heading_1(&ctx, token);
			break;
		case line_token_type_heading_2:
			token = finalise_heading_2(&ctx, token);
			break;
		case line_token_type_heading_3:
			token = finalise_heading_3(&ctx, token);
			break;
		case line_token_type_reference:
			//token = finalise_reference(&ctx, token);
			break;
		case line_token_type_preformatted:
			//token = finalise_preformatted(&ctx, token);
			break;
		case line_token_type_block_paragraph:
			token = finalise_blockquote(&ctx, token);
			break;
		case line_token_type_ordered_list_roman:
			token = finalise_ordered_list_roman(&ctx, token);
			break;
		default:
			token = finalise_get_next_token(&ctx);
		}
	}
}