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
			print_simple_text(ctx->f, reference->text);
			fprintf(ctx->f, "\">[%d]</a></sup>", chapter_ref_count + 1);
		}
		else
		{
			print_char(ctx->f, *text);
		}

		++text;
	}
}

static const char* generate_url_path(const char* filepath, const char* ext)
{
	assert(filepath);
	assert(*filepath);
	assert(ext);
	assert(*ext);
	assert(*ext != '.');

	char buffer[256];
	char* current = buffer;

	memcpy(current, output_dir, output_len);
	current += output_len;
	*current++ = '/';

	for (;;)
	{
		const char c = *filepath++;

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
	if (current == buffer)
		handle_error("Unable to generate web-safe filename from title.");

	const int64_t size = current - buffer;
	char* url_path = malloc(size);
	memcpy(url_path, buffer, size);

	return url_path;
}

static void create_html_css(void)
{
	FILE* f = open_file(OUTPUT_DIR "/style.css", file_mode_write);

	// This adds support for light and dark modes based on browser settings
	fputs(
		":root {\n\t"
			"color-scheme: light dark;\n"
		"}",
		f
	);
	fputs("\n\n", f);

	// Default universal settings
	fputs(
		"body {\n\t"
			// Set font and base size
			"font-family: \"Georgia\", serif;\n\t"
			"font-size: 18px;\n\t"
			// Set theme colours
			"color: light-dark(#000000, #E0E0E0);\n\t"
			"background-color: light-dark(#FFFFFF, #1E1E1E);\n\t"
			// Centre text at a comfortable column width for reading
			"max-width: 55ch;\n\t"
			"margin-left: auto;\n\t"
			"margin-right: auto;\n\t"
			// Add padding so we don't go up against the edges of small displays
			"padding-left: 1em;\n\t"
			"padding-right: 1em;\n\t"
			"padding-bottom: 1em;\n"
		"}",
		f
	);
	fputs("\n\n", f);

	// Chapter headings are centred
	fputs(
		"h1 {\n\t"
			"text-align: center;\n"//\t"
			//"page-break-before: always;\n" // Ensures chapters start on a new page when printed
		"}",
		f
	);
	fputs("\n\n", f);

	// Title heading
	fputs(
		"h1.title {\n\t"
			"font-size: 48px;\n\t"
			"padding-top: 128px;\n\t"
			"padding-bottom: 128px;\n\t"
			"page-break-before: avoid;\n"
		"}",
		f
	);
	fputs("\n\n", f);

	// Superscript
	fputs(
		"sup {\n\t"
			"line-height: 0;\n\t"	// Prevent references from increasing line height
			"font-size: 0.75em;\n"
		"}",
		f
	);
	fputs("\n\n", f);

	// Remove underlines from hyperlinks
	fputs(
		"a {\n\t"
			"text-decoration: none;\n"
		"}",
		f
	);
	fputs("\n\n", f);

	// Add underline when hovering over link
	fputs(
		"a:hover {\n\t"
			"text-decoration: underline;\n"
		"}",
		f
	);
	fputs("\n\n", f);

	// Links should not stand out when printing
	fputs(
		"@media print {\n\t"
			"a {\n\t\t"
				"color: black;\n\t"
			"}\n"
		"}",
		f
	);
	fputs("\n\n", f);

	// Avoid new lines after a heading when printing
	fputs(
		"h1, h2, h3 {\n\t"
			"page-break-after: avoid;\n"
		"}",
		f
	);
	fputs("\n\n", f);

	// Paragraphs which don't follow a heading are not indented
	fputs(
		"p {\n\t"
			"margin-top: 0;\n\t"
			"text-indent: 1.5em;\n\t"
			"text-align: justify;\n\t"
			"hyphens: auto;\n\t"
			"margin-bottom: 0;\n"
		"}",
		f
	);
	fputs("\n\n", f);

	// Paragraph with previous gap
	fputs(
		"p.paragraph-break {\n\t"
			"margin-top: 1em;\n\t"
			"text-indent: 0;\n"
		"}",
		f
	);
	fputs("\n\n", f);

	// Authors
	fputs(
		"p.authors {\n\t"
			"text-align: center;\n\t"
			"padding-top: 0;\n\t"
			"padding-bottom: 128px;\n\t"
			"text-indent: 0;\n"
		"}",
		f
	);
	fputs("\n\n", f);

	// Footnote
	fputs(
		"p.footnote {\n\t"
			"margin-top: 1em;\n\t"
			"text-indent: 0;\n\t"
			"font-size: 0.75em;\n"
		"}",
		f
	);
	fputs("\n\n", f);

	// Paragraphs after headings are not indented
	fputs(
		"h1 + p,\n"
		"h2 + p,\n"
		"h3 + p {\n\t"
			"text-indent: 0;\n"
		"}",
		f
	);
	fputs("\n\n", f);

	// Blockquote indentation
	fputs(
		"blockquote {\n\t"
			"margin-left: 1.5em;\n"
		"}",
		f
	);
	fputs("\n\n", f);

	// First paragraphs within a blockquote are not indented
	fputs(
		"blockquote p {\n\t"
			"text-indent: 0;\n"
		"}",
		f
	);
	fputs("\n\n", f);

	// Paragraphs following first blockquote paragraph are indented
	fputs(
		"blockquote p + p {\n\t"
			"text-indent: 1.5em;\n"
		"}",
		f
	);
	fputs("\n\n", f);

	// Paragraphs following blockquotes are not indented
	fputs(
		"blockquote + p {\n\t"
			"text-indent: 0;\n"
		"}",
		f
	);
	fputs("\n\n", f);

	// Lists
	fputs(
		"ol, ul {\n\t"
			"text-align: justify;\n\t"
			"hyphens: auto;\n\t"
			"margin-left: 1.5em;\n\t"
			"padding-left: 0;\n"
		"}",
		f
	);
	fputs("\n\n", f);

	// Paragraphs following lists are not be indented
	fputs(
		"ol + p,\n"
		"ul + p {\n\t"
			"text-indent: 0;\n"
		"}",
		f
	);
	fputs("\n\n", f);

	// Chapter list should be left-aligned
	fputs(
		"ul.chapters {\n\t"
			"text-align: left;\n"
		"}",
		f
	);

	fclose(f);
}

static void generate_html(const document* doc)
{
	create_html_css();

	const char* filepath = generate_url_path(doc->metadata.title, "html");
	FILE* f = open_file(filepath, file_mode_write);

	html_context ctx = {
		.f		= f,
		.doc	= doc
	};

	fputs(
		"<!DOCTYPE html>\n"
		"<html lang=\"en-GB\">\n"
		"\t<head>\n"
		"\t\t<meta charset=\"UTF-8\">\n"
		"\t\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
		"\t\t<link href=\"style.css\" rel=\"stylesheet\">\n",
		f
	);

	if (doc->metadata.title)
		fprintf(f, "\t\t<title>%s</title>\n", doc->metadata.title);

	fputs(
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
				fprintf(f, "\n\t\t<p class=\"footnote\" id=\"ref%d\">\n", ctx.ref_count);
				fprintf(f, "\t\t\t[<a href=\"#ref-return%d\">%d</a>] ", ctx.ref_count, ctx.chapter_ref_count);
				print_html_text_block(&ctx, reference->text);
				fprintf(f, "\n\t\t</p>");
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