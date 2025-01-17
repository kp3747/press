static void print_html_text_block(html_context* ctx, const char* text)
{
	while (*text)
	{
		if (*text == text_token_type_strong_begin)
		{
			print_str(ctx->f, "<strong>");
		}
		else if (*text == text_token_type_strong_end)
		{
			print_str(ctx->f, "</strong>");
		}
		else if (*text == text_token_type_emphasis_begin)
		{
			print_str(ctx->f, "<em>");
		}
		else if (*text == text_token_type_emphasis_end)
		{
			print_str(ctx->f, "</em>");
		}
		else if (*text == text_token_type_note)
		{
			const uint32_t note_count = ctx->inline_note_count++ + 1;
			const uint32_t chapter_note_count = ctx->inline_chapter_note_count++;

			const document_chapter* chapter = &ctx->doc->chapters[ctx->chapter_index];
			const document_note* note = &chapter->notes[chapter_note_count];

			print_fmt(ctx->f, "<sup><a id=\"note-return%d\" href=\"#note%d\" title=\"", note_count, note_count);

			int begin_count = 0;
			for (uint32_t element_index = 0; element_index < note->element_count; ++element_index)
			{
				document_element* element = &note->elements[element_index];

				if (element->type == document_element_type_text_block)
				{
					print_simple_text(ctx->f, element->text);
				}
				else if (element->type == document_element_type_paragraph_begin)
				{
					++begin_count;
					if (begin_count > 1)
						print_str(ctx->f, "\n\n");
				}
			}

			print_fmt(ctx->f, "\">[%d]</a></sup>", chapter_note_count + 1);
		}
		else
		{
			print_char_token(ctx->f, *text);
		}

		++text;
	}
}

static const char* generate_url_path(const char* title, const char* ext)
{
	assert(title);
	assert(*title);
	assert(ext);
	assert(*ext);
	assert(*ext != '.');

	// Allocate worst-case size (output_dir + '/' + title + '.' + extension + null terminator)
	const int64_t buffer_size = output_len + 1 + strlen(title) + 1 + strlen(ext) + 1;
	char* buffer = mem_alloc(buffer_size);

	// Copy output directory path
	char* current = buffer;
	memcpy(current, output_dir, output_len);
	current += output_len;
	*current++ = '/';

	// Normalise title to a web-safe filename
	const char* title_begin = current;
	for (;;)
	{
		const char c = *title++;

		if (c >= 'a' && c <= 'z')
			*current++ = c;
		else if (c >= 'A' && c <= 'Z')
			*current++ = c + 32;
		else if (c == '.')
			*current++ = '.';
		else if (c == '-')
			*current++ = '-';
		else if (c == ' ')
			*current++ = '-';
		else if (c == 0)
			break;
	}

	// Add extension
	*current++ = '.';
	while (*ext)
		*current++ = *ext++;

	// Null terminate
	*current++ = 0;

	// TODO: Add metadata option to override filename
	if (current == title_begin)
		handle_error("Unable to generate web-safe filename from title.");

	return buffer;
}

static void generate_html(const document* doc)
{
	// Filepath mem is temporary
	void* frame = mem_push();
	const char* filepath = generate_url_path(doc->metadata.title, "html");
	file f = open_file(filepath, file_mode_write);
	mem_pop(frame);

	html_context ctx = {
		.f		= f,
		.doc	= doc
	};

	print_str(f,
		"<!DOCTYPE html>\n"
		"<html lang=\"en-GB\">\n"
		"	<head>\n"
		"		<meta charset=\"UTF-8\">\n"
		"		<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
	);

	if (doc->metadata.title)
	{
		print_str(f, "\t\t<title>");
		print_simple_text(f, doc->metadata.title);
		print_str(f, "</title\n>");
	}

	print_str(f, "\t\t<style>\n");
	static void output_css(html_context* ctx);
	output_css(&ctx);
	print_str(f, "\t\t</style>\n\t</head>\n\t<body>");

	if (doc->metadata.type == document_type_book)
	{
		if (doc->metadata.title)
			print_fmt(f, "\n\t\t<h1 class=\"title\">%s</h1>", doc->metadata.title);

		if (doc->metadata.author_count)
		{
			print_str(f, "\n\t\t<p class=\"authors\">");

			for (uint32_t i = 0; i < doc->metadata.author_count - 1; ++i)
				print_fmt(f, "\n\t\t\t%s<br>", doc->metadata.authors[i]);
			print_fmt(f, "\n\t\t\t%s", doc->metadata.authors[doc->metadata.author_count - 1]);

			print_str(f, "\n\t\t</p>");
		}

		if (doc->metadata.translator_count)
		{
			print_str(f, "\n\t\t<p class=\"authors\">\n\t\t\tTranslated by:<br>");

			for (uint32_t i = 0; i < doc->metadata.translator_count - 1; ++i)
				print_fmt(f, "\n\t\t\t%s<br>", doc->metadata.translators[i]);
			print_fmt(f, "\n\t\t\t%s", doc->metadata.translators[doc->metadata.translator_count - 1]);

			print_str(f, "\n\t\t</p>");
		}

		if (doc->chapter_count > 1)
		{
			print_str(f, "\n\t\t<h1>Contents</h1>\n\t\t<ul class=\"chapters\">\n");

			for (uint32_t chapter_index = 0; chapter_index < doc->chapter_count; ++chapter_index)
			{
				document_element* heading = doc->chapters[chapter_index].elements;

				print_fmt(f, "\t\t\t<li><a href=\"#h%d\">", chapter_index + 1);
				print_simple_text(f, heading->text);
				print_str(f, "</a></li>\n");
			}

			print_str(f, "\t\t</ul>");
		}
	}

	int depth = 2;

	for (uint32_t chapter_index = 0; chapter_index < doc->chapter_count; ++chapter_index)
	{
		document_chapter* chapter = &doc->chapters[chapter_index];
		ctx.chapter_index = chapter_index;

		for (uint32_t element_index = 0; element_index < chapter->element_count; ++element_index)
		{
			document_element* element = &chapter->elements[element_index];

			switch (element->type)
			{
			case document_element_type_dinkus:
				print_tabs(f, depth);
				print_str(f, "<hr>");
				break;
			case document_element_type_heading_1:
				ctx.chapter_note_count = 0;
				ctx.inline_chapter_note_count = 0;

				print_tabs(f, depth);
				if (doc->metadata.type == document_type_book)
					print_fmt(f, "<h1 id=\"h%d\">", chapter_index + 1);
				else
					print_str(f, "<h1>");
				print_html_text_block(&ctx, element->text);
				print_str(f, "</h1>");
				break;
			case document_element_type_heading_2:
				print_tabs(f, depth);
				print_str(f, "<h2>");
				print_html_text_block(&ctx, element->text);
				print_str(f, "</h2>");
				break;
			case document_element_type_heading_3:
				print_tabs(f, depth);
				print_str(f, "<h3>");
				print_html_text_block(&ctx, element->text);
				print_str(f, "</h3>");
				break;
			case document_element_type_text_block:
				print_html_text_block(&ctx, element->text);
				break;
			case document_element_type_line_break:
				print_str(f, "<br>");
				break;
			case document_element_type_paragraph_begin:
				print_tabs(f, depth);
				print_str(f, "<p>");
				break;
			case document_element_type_paragraph_break_begin:
				print_tabs(f, depth);
				print_str(f, "<p class=\"paragraph-break\">");
				break;
			case document_element_type_paragraph_end:
				print_str(f, "</p>");
				break;
			case document_element_type_blockquote_begin:
				print_tabs(f, depth++);
				print_str(f, "<blockquote>");
				break;
			case document_element_type_blockquote_end:
				print_tabs(f, --depth);
				print_str(f, "</blockquote>");
				break;
			case document_element_type_blockquote_citation:
				print_tabs(f, depth);
				print_str(f, "<p class=\"paragraph-break\">");
				print_em_dash(f);
				print_html_text_block(&ctx, element->text);
				print_str(f, "</p>");
				break;
			case document_element_type_right_aligned_begin:
				print_tabs(f, depth);
				print_str(f, "<p class=\"right-aligned\">");
				break;
			case document_element_type_ordered_list_begin_roman:
				print_tabs(f, depth++);
				print_str(f, "<ol type=\"I\">");
				break;
			case document_element_type_ordered_list_begin_arabic:
				print_tabs(f, depth++);
				print_str(f, "<ol>");
				break;
			case document_element_type_ordered_list_begin_letter:
				print_tabs(f, depth++);
				print_str(f, "<ol type=\"a\">");
				break;
			case document_element_type_ordered_list_end:
				print_tabs(f, --depth);
				print_str(f, "</ol>");
				break;
			case document_element_type_unordered_list_begin:
				print_tabs(f, depth++);
				print_str(f, "<ul>");
				break;
			case document_element_type_unordered_list_end:
				print_tabs(f, --depth);
				print_str(f, "</ul>");
				break;
			case document_element_type_list_item:
				print_tabs(f, depth);
				print_str(f, "<li>");
				print_html_text_block(&ctx, element->text);
				print_str(f, "</li>");
				break;
			}
		}

		if (chapter->note_count > 0)
		{
			for (uint32_t note_index = 0; note_index < chapter->note_count; ++note_index)
			{
				++ctx.note_count;
				++ctx.chapter_note_count;

				document_note* note = &chapter->notes[note_index];

				for (uint32_t element_index = 0; element_index < note->element_count; ++element_index)
				{
					document_element* element = &note->elements[element_index];

					switch (element->type)
					{
					case document_element_type_text_block:
						print_html_text_block(&ctx, element->text);
						break;
					case document_element_type_line_break:
						print_str(f, "<br>");
						break;
					case document_element_type_paragraph_begin:
						print_tabs(f, depth);
						if (element_index == 0)
						{
							print_fmt(f, "<p class=\"footnote\" id=\"note%d\">", ctx.note_count);
							print_fmt(f, "[<a href=\"#note-return%d\">%d</a>] ", ctx.note_count, ctx.chapter_note_count);
						}
						else
						{
							print_str(f, "<p class=\"footnote_paragraph\">");
						}
						break;
					case document_element_type_paragraph_break_begin:
						print_tabs(f, depth);
						print_str(f, "<p class=\"paragraph-break\">");
						break;
					case document_element_type_paragraph_end:
						print_str(f, "</p>");
						break;
					}
				}
			}
		}
	}

	print_str(f, "\n\t</body>\n</html>");

	close_file(f);
}