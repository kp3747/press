static void print_html_text_block(html_context* ctx, const char* text)
{
	while (*text)
	{
		if (*text == text_token_type_strong_begin)
		{
			fputs("<strong>", ctx->f);
		}
		else if (*text == text_token_type_strong_end)
		{
			fputs("</strong>", ctx->f);
		}
		else if (*text == text_token_type_emphasis_begin)
		{
			fputs("<em>", ctx->f);
		}
		else if (*text == text_token_type_emphasis_end)
		{
			fputs("</em>", ctx->f);
		}
		else if (*text == text_token_type_reference)
		{
			const uint32_t ref_count = ctx->inline_ref_count++ + 1;
			const uint32_t chapter_ref_count = ctx->inline_chapter_ref_count++;

			const document_chapter* chapter = &ctx->doc->chapters[ctx->chapter_index];
			const document_reference* reference = &chapter->references[chapter_ref_count];

			fprintf(ctx->f, "<sup><a id=\"ref-return%d\" href=\"#ref%d\" title=\"", ref_count, ref_count);

			int begin_count = 0;
			for (uint32_t element_index = 0; element_index < reference->element_count; ++element_index)
			{
				document_element* element = &reference->elements[element_index];

				if (element->type == document_element_type_text_block)
				{
					print_simple_text(ctx->f, element->text);
				}
				else if (element->type == document_element_type_paragraph_begin)
				{
					++begin_count;
					if (begin_count > 1)
						fputs("\n\n", ctx->f);
				}
			}

			fprintf(ctx->f, "\">[%d]</a></sup>", chapter_ref_count + 1);
		}
		else
		{
			print_char(ctx->f, *text);
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
	FILE* f = open_file(filepath, file_mode_write);
	mem_pop(frame);

	html_context ctx = {
		.f		= f,
		.doc	= doc
	};

	fputs(
		"<!DOCTYPE html>\n"
		"<html lang=\"en-GB\">\n"
		"	<head>\n"
		"		<meta charset=\"UTF-8\">\n"
		"		<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n",
		f
	);

	if (doc->metadata.title)
	{
		fputs("\t\t<title>", f);
		// TODO: Text tokens are not currently supported, but this will work once they are
		print_simple_text(f, doc->metadata.title);
		fputs("</title\n>", f);
	}

	fputs("\t\t<style>\n", f);
	static void output_css(html_context* ctx);
	output_css(&ctx);
	fputs(
		"\t\t</style>\n"
		"\t</head>\n"
		"\t<body>",
		f
	);

	if (doc->metadata.type == document_type_book)
	{
		if (doc->metadata.title)
			fprintf(f, "\n\t\t<h1 class=\"title\">%s</h1>", doc->metadata.title);

		if (doc->metadata.author_count)
		{
			fprintf(f, "\n\t\t<p class=\"authors\">");

			for (uint32_t i = 0; i < doc->metadata.author_count - 1; ++i)
				fprintf(f, "\n\t\t\t%s<br>", doc->metadata.authors[i]);
			fprintf(f, "\n\t\t\t%s", doc->metadata.authors[doc->metadata.author_count - 1]);

			fprintf(f, "\n\t\t</p>");
		}

		if (doc->metadata.translator_count)
		{
			fprintf(f, "\n\t\t<p class=\"authors\">");
			fprintf(f, "\n\t\t\tTranslated by:<br>");

			for (uint32_t i = 0; i < doc->metadata.translator_count - 1; ++i)
				fprintf(f, "\n\t\t\t%s<br>", doc->metadata.translators[i]);
			fprintf(f, "\n\t\t\t%s", doc->metadata.translators[doc->metadata.translator_count - 1]);

			fprintf(f, "\n\t\t</p>");
		}

		if (doc->chapter_count > 1)
		{
			fprintf(f,
				"\n\t\t<h1>Contents</h1>\n"
				"\t\t<p>\n"
				"\t\t\t<ul class=\"chapters\">\n"
			);

			for (uint32_t chapter_index = 0; chapter_index < doc->chapter_count; ++chapter_index)
			{
				document_element* heading = doc->chapters[chapter_index].elements;

				fprintf(f, "\t\t\t\t<li><a href=\"#h%d\">", chapter_index + 1);
				print_simple_text(ctx.f, heading->text);
				fprintf(f, "</a></li>\n");
			}

			fprintf(f,
				"\t\t\t</ul>\n"
				"\t\t</p>"
			);
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
				fputs("<hr>", f);
				break;
			case document_element_type_heading_1:
				ctx.chapter_ref_count = 0;
				ctx.inline_chapter_ref_count = 0;

				print_tabs(f, depth);
				if (doc->metadata.type == document_type_book)
					fprintf(f, "<h1 id=\"h%d\">", chapter_index + 1);
				else
					fputs("<h1>", f);
				print_html_text_block(&ctx, element->text);
				fprintf(f, "</h1>");
				break;
			case document_element_type_heading_2:
				print_tabs(f, depth);
				fputs("<h2>", f);
				print_html_text_block(&ctx, element->text);
				fprintf(f, "</h2>");
				break;
			case document_element_type_heading_3:
				print_tabs(f, depth);
				fputs("<h3>", f);
				print_html_text_block(&ctx, element->text);
				fprintf(f, "</h3>");
				break;
			case document_element_type_text_block:
				print_html_text_block(&ctx, element->text);
				break;
			case document_element_type_line_break:
				fputs("<br>", f);
				break;
			case document_element_type_paragraph_begin:
				print_tabs(f, depth);
				fputs("<p>", f);
				break;
			case document_element_type_paragraph_break_begin:
				print_tabs(f, depth);
				fputs("<p class=\"paragraph-break\">", f);
				break;
			case document_element_type_paragraph_end:
				fputs("</p>", f);
				break;
			case document_element_type_blockquote_begin:
				print_tabs(f, depth++);
				fputs("<blockquote>", f);
				break;
			case document_element_type_blockquote_end:
				print_tabs(f, --depth);
				fputs("</blockquote>", f);
				break;
			case document_element_type_blockquote_citation:
				print_tabs(f, depth);
				fputs("<p class=\"paragraph-break\">", f);
				print_em_dash(f);
				print_html_text_block(&ctx, element->text);
				fputs("</p>", f);
				break;
			case document_element_type_right_aligned_begin:
				print_tabs(f, depth);
				fputs("<p class=\"right-aligned\">", f);
				break;
			case document_element_type_ordered_list_begin_roman:
				print_tabs(f, depth++);
				fputs("<ol type=\"I\">", f);
				break;
			case document_element_type_ordered_list_begin_arabic:
				print_tabs(f, depth++);
				fputs("<ol>", f);
				break;
			case document_element_type_ordered_list_begin_letter:
				print_tabs(f, depth++);
				fputs("<ol type=\"a\">", f);
				break;
			case document_element_type_ordered_list_end:
				print_tabs(f, --depth);
				fputs("</ol>", f);
				break;
			case document_element_type_unordered_list_begin:
				print_tabs(f, depth++);
				fputs("<ul>", f);
				break;
			case document_element_type_unordered_list_end:
				print_tabs(f, --depth);
				fputs("</ul>", f);
				break;
			case document_element_type_list_item:
				print_tabs(f, depth);
				fputs("<li>", f);
				print_html_text_block(&ctx, element->text);
				fprintf(f, "</li>");
				break;
			}
		}

		if (chapter->reference_count > 0)
		{
			for (uint32_t reference_index = 0; reference_index < chapter->reference_count; ++reference_index)
			{
				++ctx.ref_count;
				++ctx.chapter_ref_count;

				document_reference* reference = &chapter->references[reference_index];

				for (uint32_t element_index = 0; element_index < reference->element_count; ++element_index)
				{
					document_element* element = &reference->elements[element_index];

					switch (element->type)
					{
					case document_element_type_text_block:
						print_html_text_block(&ctx, element->text);
						break;
					case document_element_type_line_break:
						fputs("<br>", f);
						break;
					case document_element_type_paragraph_begin:
						print_tabs(f, depth);
						if (element_index == 0)
						{
							fprintf(f, "<p class=\"footnote\" id=\"ref%d\">", ctx.ref_count);
							fprintf(f, "[<a href=\"#ref-return%d\">%d</a>] ", ctx.ref_count, ctx.chapter_ref_count);
						}
						else
						{
							fputs("<p class=\"footnote_paragraph\">", f);
						}
						break;
					case document_element_type_paragraph_break_begin:
						print_tabs(f, depth);
						fputs("<p class=\"paragraph-break\">", f);
						break;
					case document_element_type_paragraph_end:
						fputs("</p>", f);
						break;
					}
				}
			}
		}
	}

	fputs(
		"\n\t</body>\n"
		"</html>",
		f
	);

	fclose(f);
}