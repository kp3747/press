static void create_epub_mimetype(void)
{
	file f = open_file(OUTPUT_DIR "/epub/mimetype", file_mode_write);
	print_str(f, "application/epub+zip");
	close_file(f);
}

static void create_epub_meta_inf(void)
{
	file f = open_file(OUTPUT_DIR "/epub/META-INF/container.xml", file_mode_write);

	print_str(f,
		"<?xml version=\"1.0\"?>\n"
		"<container version=\"1.0\" xmlns=\"urn:oasis:names:tc:opendocument:xmlns:container\">\n"
		"	<rootfiles>\n"
		"		<rootfile full-path=\"content.opf\" media-type=\"application/oebps-package+xml\" />\n"
		"	</rootfiles>\n"
		"</container>"
	);

	close_file(f);
}

static void create_epub_css(void)
{
	file f = open_file(OUTPUT_DIR "/epub/style.css", file_mode_write);

	print_str(f,
		// Chapter headings are centred
		"h1 {\n"
		"	text-align: center;\n"
		"}\n\n"
		// Paragraphs which don't follow a heading are not indented
		"p {\n"
		"	margin-top: 0;\n"
		"	text-indent: 1.5em;\n"
		"	hyphens: auto;\n"
		"	margin-bottom: 0;\n"
		"}\n\n"
		// Right-aligned paragraphs
		"p.right-aligned {\n"
		"	margin-top: 1em;\n"
		"	text-align: right;\n"
		"}\n\n"
		// Paragraph with previous gap
		".paragraph-break {\n"
		"	margin-top: 1em;\n"
		"	text-indent: 0;\n"
		"}\n\n"
		// First footnote paragraph
		"p.footnote {\n"
		"	margin-top: 1.5em;\n"
		"	text-indent: 0;\n"
		"	font-size: 0.75em;\n"
		"}\n\n"
		// Footnote paragraph
		"p.footnote_paragraph {\n"
		"	text-indent: 1.5em;\n"
		"	font-size: 0.75em;\n"
		"}\n\n"
		// Footnote following another footnote
		"p.footnote_paragraph + p.footnote {\n"
			"margin-top: 1em;\n"
		"}\n\n"
		"p.footnote + p.footnote {\n"
			"margin-top: 1em;\n"
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
		"div.dinkus + p {\n"
		"	text-indent: 0;\n"
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
		// Dinkus
		"div.dinkus {\n"
		"	text-align: center;\n"
		"	word-spacing: 1em;\n"
		"	margin-top: 1.5em;\n"
		"	margin-bottom: 1.5em;\n"
		"	user-select: none;\n"
		"}"
	);

	close_file(f);
}

static void create_epub_opf(const document* doc)
{
	file f = open_file(OUTPUT_DIR "/epub/content.opf", file_mode_write);

	print_str(f,
		"<?xml version=\"1.0\"?>\n"
		"<package version=\"2.0\" xmlns=\"http://www.idpf.org/2007/opf\" unique-identifier=\"bookid\">\n"
		"	<metadata xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:opf=\"http://www.idpf.org/2007/opf\">\n"
	);

	print_fmt(f, "\t\t<dc:title>%s</dc:title>\n", doc->metadata.title);
	print_str(f, "\t\t<dc:language>en-GB</dc:language>\n");

	// TODO: Generate UUID
	print_str(f, "\t\t<dc:identifier id=\"bookid\" opf:scheme=\"uuid\">6519aff0-f47c-437c-ba20-cc0782b39b05</dc:identifier>\n");

	for (uint32_t i = 0; i < doc->metadata.author_count; ++i)
		print_fmt(f, "\t\t<dc:creator>%s</dc:creator>\n", doc->metadata.authors[i]);

	// TODO: Figure out how to add translator metadata
//	for (uint32_t i = 0; i < doc->metadata.translator_count; ++i)
//		fprintf(f, "\t\t<dc:creator opf:role=\"translator\">%s</dc:creator>\n", doc->metadata.translators[i]);

	// TODO: Add metadata to specify cover image
	//fputs("\t\t<meta name=\"cover\" content=\"cover_image\"/>\n", f);

	print_str(f,
		"	</metadata>\n"
		"	<manifest>\n"
		"		<item id=\"ncx\" href=\"toc.ncx\" media-type=\"application/x-dtbncx+xml\"/>\n"
		"		<item id=\"css\" href=\"style.css\" media-type=\"text/css\"/>\n"
		"		<item id=\"cover\" href=\"cover.xhtml\" media-type=\"application/xhtml+xml\"/>\n"
	);

//	fprintf(f, "\t\t<item id=\"cover_image\" href=\"cover.png\" media-type=\"image/png\"/>\n");

	if (doc->chapter_count > 1)
		print_fmt(f, "\t\t<item id=\"toc\" href=\"toc.xhtml\" media-type=\"application/xhtml+xml\"/>\n");

	for (uint32_t i = 0; i < doc->chapter_count; ++i)
		print_fmt(f, "\t\t<item id=\"chapter%d\" href=\"chapter%d.xhtml\" media-type=\"application/xhtml+xml\"/>\n", i + 1, i + 1);

	print_str(f,
		"	</manifest>\n"
		"	<spine toc=\"ncx\">\n"
		"		<itemref idref=\"cover\"/>\n"
	);

	if (doc->chapter_count > 1)
		print_str(f, "\t\t<itemref idref=\"toc\"/>\n");

	for (uint32_t i = 0; i < doc->chapter_count; ++i)
		print_fmt(f, "\t\t<itemref idref=\"chapter%d\"/>\n", i + 1);

	print_str(f,
		"	</spine>\n"
		"</package>"
	);

	close_file(f);
}

static void create_epub_ncx(const document* doc)
{
	file f = open_file(OUTPUT_DIR "/epub/toc.ncx", file_mode_write);

	print_str(f,
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
		"<ncx xmlns=\"http://www.daisy.org/z3986/2005/ncx/\" version=\"2005-1\">\n"
		"	<head>\n"
		"		<meta name=\"dtb:uid\" content=\"6519aff0-f47c-437c-ba20-cc0782b39b05\"/>\n"
		"	</head>\n"
	);

	print_str(f, "\t<docTitle>");
	print_fmt(f, "\t\t<text>%s</text>\n", doc->metadata.title);
	print_str(f, "\t</docTitle>\t<navMap>\n");

	for (uint32_t i = 0; i < doc->chapter_count; ++i)
	{
		document_element* heading = doc->chapters[i].elements;

		print_fmt(f, "\t\t<navPoint class=\"chapter\" id=\"chapter%d\" playOrder=\"%d\">\n", i + 1, i + 1);
		print_str(f, "\t\t\t<navLabel>\n");
		print_fmt(f, "\t\t\t\t<text>%s</text>\n", heading->text);
		print_str(f, "\t\t\t</navLabel>\n");
		print_fmt(f, "\t\t\t<content src=\"chapter%d.xhtml\"/>\n", i + 1);
		print_str(f, "\t\t</navPoint>\n");
	}

	print_str(f, "\t</navMap>\n</ncx>");

	close_file(f);
}

static void create_epub_cover(const document* doc, const char* cover)
{
	if (cover)
	{
		void* frame = mem_push();

		// Load file
		file src = open_file(cover, file_mode_read);
		const uint32_t size = get_file_size(src);
		char* data = mem_alloc(size);
		read_file(src, data, size);
		close_file(src);

		// Save file
		file dst = open_file(OUTPUT_DIR "/epub/cover.xhtml", file_mode_write);
		write_file(dst, data, size);
		close_file(dst);

		mem_pop(frame);

		return;
	}

	file f = open_file(OUTPUT_DIR "/epub/cover.xhtml", file_mode_write);

	// TODO: Title should be formatted
	print_fmt(f,
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
		"<html xmlns=\"http://www.w3.org/1999/xhtml\">\n\t"
			"<head>\n\t\t"
				"<title>Cover</title>\n\t\t"
				"<style>\n"
		"body {\n\t"
			"text-align: center;\n"
		"}\n\t\t"
				"</style>\n\t"
			"</head>\n\t"
			"<body>\n\t\t"
				"<h1>%s</h1>",
		doc->metadata.title
	);

	if (doc->metadata.author_count)
	{
		print_str(f, "\n\t\t<p>");

		for (uint32_t i = 0; i < doc->metadata.author_count - 1; ++i)
			print_fmt(f, "\n\t\t\t%s<br/>", doc->metadata.authors[i]);
		print_fmt(f, "\n\t\t\t%s", doc->metadata.authors[doc->metadata.author_count - 1]);

		print_str(f, "\n\t\t</p>");
	}

	if (doc->metadata.translator_count)
	{
		print_str(f, "\n\t\t<p>");
		print_str(f, "\n\t\t\tTranslated by:<br/>");

		for (uint32_t i = 0; i < doc->metadata.translator_count - 1; ++i)
			print_fmt(f, "\n\t\t\t%s<br/>", doc->metadata.translators[i]);
		print_fmt(f, "\n\t\t\t%s", doc->metadata.translators[doc->metadata.translator_count - 1]);

		print_str(f, "\n\t\t</p>");
	}

	print_str(f, "\n\t</body>\n</html>");

	close_file(f);
}

static void create_epub_toc(const document* doc)
{
	if (doc->chapter_count < 2)
		return;

	file f = open_file(OUTPUT_DIR "/epub/toc.xhtml", file_mode_write);

	print_fmt(f,
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
		"<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
		"	<head>\n"
		"		<title>Contents</title>\n"
		"		<link href=\"style.css\" rel=\"stylesheet\" />\n"
		"	</head>\n"
		"	<body>\n"
		"		<h1>Contents</h1>\n"
		"		<ul class=\"chapters\">"
	);

	for (uint32_t chapter_index = 0; chapter_index < doc->chapter_count; ++chapter_index)
	{
		document_element* heading = doc->chapters[chapter_index].elements;

		print_fmt(f, "\t\t\t<li><a href=\"chapter%d.xhtml\">", chapter_index + 1);
		print_simple_text(f, heading->text);
		print_str(f, "</a></li>\n");
	}

	print_str(f,
		"		</ul>\n"
		"	</body>\n"
		"</html>"
	);

	close_file(f);
}

static void create_epub_chapter(const document* doc, uint32_t index)
{
	const char* filepath = generate_path(OUTPUT_DIR "/epub/chapter%d.xhtml", index + 1);
	file f = open_file(filepath, file_mode_write);

	html_context ctx = {
		.f				= f,
		.doc			= doc,
		.chapter_index	= index
	};

	document_chapter* chapter = &doc->chapters[index];

	print_fmt(f,
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
		"<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
		"	<head>\n"
		"		<title>%s</title>\n"
		"		<link href=\"style.css\" rel=\"stylesheet\" />\n"
		"	</head>\n"
		"	<body>",
		chapter->elements[0].text
	);

	int depth = 2;

	for (uint32_t element_index = 0; element_index < chapter->element_count; ++element_index)
	{
		document_element* element = &chapter->elements[element_index];

		switch (element->type)
		{
		case document_element_type_dinkus:
			print_tabs(f, depth);
			print_str(f, "<div class=\"dinkus\">* * *</div>");
			break;
		case document_element_type_heading_1:
			ctx.chapter_ref_count = 0;
			ctx.inline_chapter_ref_count = 0;

			print_tabs(f, depth);
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
			print_tabs(f, depth + 1);
			print_html_text_block(&ctx, element->text);
			break;
		case document_element_type_line_break:
			print_str(f, "<br/>");
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
			print_tabs(f, depth);
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
					print_str(f, "<br/>");
					break;
				case document_element_type_paragraph_begin:
					print_tabs(f, depth);
					if (element_index == 0)
					{
						print_fmt(f, "\n\t\t<p class=\"footnote\" id=\"ref%d\">\n", ctx.ref_count);
						print_fmt(f, "\t\t\t[<a href=\"#ref-return%d\">%d</a>] ", ctx.ref_count, ctx.chapter_ref_count);
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

	print_str(f, "\n\t</body>\n</html>");

	close_file(f);
}

static void generate_epub_zip(const document* doc)
{
	// Allocations within this function are temporary and do not outlive the function lifetime
	void* frame = mem_push();

	uint32_t file_count = 6;
	file_count += doc->chapter_count;
	if (doc->chapter_count > 1)
		++file_count;
	const int64_t array_size = sizeof(const char**) * file_count;

	const char** inputs = mem_alloc(array_size);
	const char** outputs = mem_alloc(array_size);

	inputs[0] = OUTPUT_DIR "/epub/mimetype";
	outputs[0] = "mimetype";

	inputs[1] = OUTPUT_DIR "/epub/META-INF/container.xml";
	outputs[1] = "META-INF/container.xml";

	inputs[2] = OUTPUT_DIR "/epub/style.css";
	outputs[2] = "style.css";

	inputs[3] = OUTPUT_DIR "/epub/content.opf";
	outputs[3] = "content.opf";

	inputs[4] = OUTPUT_DIR "/epub/toc.ncx";
	outputs[4] = "toc.ncx";

	inputs[5] = OUTPUT_DIR "/epub/cover.xhtml";
	outputs[5] = "cover.xhtml";

	inputs[6] = OUTPUT_DIR "/epub/chapter1.xhtml";
	outputs[6] = "chapter1.xhtml";

	if (doc->chapter_count > 1)
	{
		inputs[7] = OUTPUT_DIR "/epub/toc.xhtml";
		outputs[7] = "toc.xhtml";

		for (uint32_t i = 1; i < doc->chapter_count; ++i)
		{
			inputs[7 + i] = generate_path(OUTPUT_DIR "/epub/chapter%d.xhtml", i + 1);
			outputs[7 + i] = generate_path("chapter%d.xhtml", i + 1);
		}
	}

	const char* epub_path = generate_path(OUTPUT_DIR "/%s.epub", doc->metadata.title);
	generate_zip(epub_path, inputs, outputs, file_count);

	mem_pop(frame);
}

static void generate_epub(const document* doc, const char* cover)
{
	void* frame = mem_push();

	create_dir(OUTPUT_DIR "\\epub");
	create_dir(OUTPUT_DIR "\\epub\\META-INF");

	create_epub_mimetype();
	create_epub_meta_inf();
	create_epub_css();
	create_epub_opf(doc);
	create_epub_ncx(doc);
	create_epub_cover(doc, cover);
	create_epub_toc(doc);

	for (uint32_t i = 0; i < doc->chapter_count; ++i)
		create_epub_chapter(doc, i);

	generate_epub_zip(doc);

	delete_dir(OUTPUT_DIR "\\epub");
}