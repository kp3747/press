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

static void output_css(html_context* ctx)
{
	// Default universal settings
	fputs(
		// This adds support for light and dark modes based on browser settings
		":root {\n"
		"	color-scheme: light dark;\n"
		"}\n\n"

		"body {\n"
		// Set font and base size
		"	font-family: 'Georgia', serif;\n"
		"	font-size: 18px;\n"
		// Set theme colours
		"	color: light-dark(#000000, #E0E0E0);\n"
		"	background-color: light-dark(#FFFFFF, #1E1E1E);\n"
		// Centre text at a comfortable column width for reading
		"	max-width: 55ch;\n"
		"	margin-left: auto;\n"
		"	margin-right: auto;\n"
		// Add padding so we don't go up against the edges of small displays
		"	padding-left: 1em;\n"
		"	padding-right: 1em;\n"
		"	padding-bottom: 1em;\n"
		"}\n\n"

		// Chapter headings are centred
		"h1 {\n"
		"	text-align: center;\n"
		"}\n\n"

		// Title heading
		"h1.title {\n"
		"	font-size: 48px;\n"
		"	padding-top: 128px;\n"
		"	padding-bottom: 128px;\n"
		"}\n\n"

		// Superscript
		"sup {\n"
		"	line-height: 0;\n"		// Prevent references from increasing line height
		"	font-size: 0.75em;\n"
		"}\n\n"

		// Remove underlines from hyperlinks
		"a {\n"
		"	text-decoration: none;\n"
		"}\n\n"

		// Add underline when hovering over link
		"a:hover {\n"
		"	text-decoration: underline;\n"
		"}\n\n"

		// Links should not stand out when printing
		"@media print {\n"
		"	a {\n"
		"		color: black;\n"
		"	}\n"
		"}\n\n"

		// Avoid new lines after a heading when printing
		"h1, h2, h3 {\n"
		"	page-break-after: avoid;\n"
		"}\n\n"

		// Paragraphs which don't follow a heading are not indented
		"p {\n"
		"	margin-top: 0;\n"
		"	text-indent: 1.5em;\n"
		"	text-align: justify;\n"
		"	hyphens: auto;\n"
		"	margin-bottom: 0;\n"
		"}\n\n"

		// Right-aligned paragraphs
		"p.right-aligned {\n"
		"	margin-top: 1em;\n"
		"	text-align: right;\n"
		"}\n\n"

		// Paragraph with previous gap
		"p.paragraph-break {\n"
		"	margin-top: 1em;\n"
		"	text-indent: 0;\n"
		"}\n\n"

		// Authors
		"p.authors {\n"
		"	text-align: center;\n"
		"	padding-top: 0;\n"
		"	padding-bottom: 128px;\n"
		"	text-indent: 0;\n"
		"}\n\n"

		// First footnote paragraph
		"p.footnote {\n"
		"	margin-top: 1.5em;\n"
		"	text-indent: 0;\n"
		"	font-size: 0.75em;\n"
		"}\n\n"

		// Subsequent footnote paragraph
		"p.footnote_paragraph {\n"
		"	text-indent: 1.5em;\n"
		"	font-size: 0.75em;\n"
		"}\n\n"

		// Footnotes following another footnote
		"p.footnote_paragraph + p.footnote {\n"
		"	margin-top: 1em;\n"
		"}\n\n"

		"p.footnote + p.footnote {\n"
		"	margin-top: 1em;\n"
		"}\n\n"

		// Paragraphs after headings are not indented
		"h1 + p,\n"
		"h2 + p,\n"
		"h3 + p {\n"
		"	text-indent: 0;\n"
		"}\n\n"

		// Blockquote indentation
		"blockquote {\n"
		"	margin-left: 1.5em;\n"
		"}\n\n"

		// First paragraphs within a blockquote are not indented
		"blockquote p {\n"
		"	text-indent: 0;\n"
		"}\n\n"

		// Paragraphs following first blockquote paragraph are indented
		"blockquote p + p {\n"
		"	text-indent: 1.5em;\n"
		"}\n\n"

		// Paragraphs following blockquotes are not indented
		"blockquote + p {\n"
		"	text-indent: 0;\n"
		"}\n\n"

		// Paragraphs following dinkuses are not indented
		"hr + p {\n"
		"	text-indent: 0;\n"
		"}\n\n"

		// Lists
		"ol, ul {\n"
		"	text-align: justify;\n"
		"	hyphens: auto;\n"
		"	margin-left: 1.5em;\n"
		"	padding-left: 0;\n"
		"}\n\n"

		// Paragraphs following lists are not be indented
		"ol + p,\n"
		"ul + p {\n"
		"	text-indent: 0;\n"
		"}\n\n"

		// Chapter list should be left-aligned
		"ul.chapters {\n"
		"	text-align: left;\n"
		"}\n\n"

		// Turn HR tags into dinkuses ("* * *")
		"hr {\n"
		"	border: none;\n"
		"	word-spacing: 1em;\n"
		"	margin-top: 1.5em;\n"
		"	margin-bottom: 1.5em;\n"
		"}\n\n"

		"hr::before {\n"
		"	color: light-dark(#000000, #E0E0E0);\n"
		"	content: '* * *';\n"
		"	display: block;\n"
		"	text-align:center;\n"
		"}\n"
		,
		ctx->f
	);
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
		"\t<head>\n"
		"\t\t<meta charset=\"UTF-8\">\n"
		"\t\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n",
		f
	);

	if (doc->metadata.title)
		fprintf(f, "\t\t<title>%s</title>\n", doc->metadata.title);

	fputs("\t\t<style>\n", f);
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