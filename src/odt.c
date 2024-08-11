static void create_odt_mimetype(void)
{
	FILE* f = open_file(OUTPUT_DIR "/odt/mimetype", file_mode_write);
	fputs("application/vnd.oasis.opendocument.text", f);
	fclose(f);
}

static void create_odt_meta_inf(void)
{
	FILE* f = open_file(OUTPUT_DIR "/odt/META-INF/manifest.xml", file_mode_write);

	fputs(
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\" manifest:version=\"1.3\">\n"
		"\t<manifest:file-entry manifest:full-path=\"/\" manifest:version=\"1.3\" manifest:media-type=\"application/vnd.oasis.opendocument.text\"/>\n"
		"\t<manifest:file-entry manifest:full-path=\"styles.xml\" manifest:media-type=\"text/xml\"/>\n"
		"\t<manifest:file-entry manifest:full-path=\"content.xml\" manifest:media-type=\"text/xml\"/>\n"
		"</manifest:manifest>",
		f
	);

	fclose(f);
}

static void create_odt_styles(void)
{
	FILE* f = open_file(OUTPUT_DIR "/odt/styles.xml", file_mode_write);

	fputs(
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<office:document-styles xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\" xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\" xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\" office:version=\"1.3\">\n"
		"\t<office:font-face-decls>\n"
		"\t\t<style:font-face style:name=\"Georgia\" svg:font-family=\"Georgia\" style:font-family-generic=\"roman\" style:font-pitch=\"variable\"/>\n"
		"\t</office:font-face-decls>\n"
		"\t<office:styles>\n"
		"\t\t<style:style style:name=\"Standard\" style:family=\"paragraph\" style:class=\"text\">\n"
		"\t\t\t<style:text-properties style:font-name=\"Georgia\" style:font-style-name=\"Regular\" fo:language=\"en\" fo:country=\"CA\"/>\n"
		"\t\t</style:style>\n"
		"\t\t<style:style style:name=\"Heading\" style:family=\"paragraph\" style:parent-style-name=\"Standard\" style:next-style-name=\"First_Paragraph\" style:class=\"text\">\n"
		"\t\t\t<style:paragraph-properties fo:margin-top=\"0.5cm\" fo:margin-bottom=\"0.0cm\" style:contextual-spacing=\"false\" fo:keep-with-next=\"always\"/>\n"
		"\t\t\t<style:text-properties style:font-name=\"Georgia\" fo:font-weight=\"bold\"/>\n"
		"\t\t</style:style>\n"
		"\t\t<style:style style:name=\"Title\" style:family=\"paragraph\" style:parent-style-name=\"Heading\" style:master-page-name=\"First_Page\">\n"
		"\t\t\t<style:paragraph-properties fo:text-align=\"center\" fo:margin-top=\"6cm\"/>\n"
		"\t\t\t<style:text-properties fo:font-size=\"72pt\"/>\n"
		"\t\t</style:style>\n"
		"\t\t<style:style style:name=\"Heading_1\" style:display-name=\"Heading 1\" style:family=\"paragraph\" style:parent-style-name=\"Heading\" style:master-page-name=\"Standard\">\n"
		"\t\t\t<style:paragraph-properties fo:text-align=\"center\" fo:margin-top=\"0.0cm\"/>\n"
		"\t\t\t<style:text-properties fo:font-size=\"18pt\"/>\n"
		"\t\t</style:style>\n"
		"\t\t<style:style style:name=\"Heading_2\" style:display-name=\"Heading 2\" style:family=\"paragraph\" style:parent-style-name=\"Heading\">\n"
		"\t\t\t<style:text-properties fo:font-size=\"14pt\"/>\n"
		"\t\t</style:style>\n"
		"\t\t<style:style style:name=\"Heading_3\" style:display-name=\"Heading 3\" style:family=\"paragraph\" style:parent-style-name=\"Heading\">\n"
		"\t\t\t<style:text-properties fo:font-size=\"12pt\"/>\n"
		"\t\t</style:style>\n"
		"\t\t<style:style style:name=\"Paragraph\" style:family=\"paragraph\" style:parent-style-name=\"Standard\">\n"
		"\t\t\t<style:paragraph-properties fo:margin-top=\"0cm\" fo:margin-bottom=\"0cm\" fo:line-height=\"115%\" fo:text-align=\"justify\" style:justify-single-word=\"false\" fo:hyphenation-ladder-count=\"no-limit\" fo:text-indent=\"0cm\" fo:text-align-last=\"start\"/>\n"
		"\t\t\t<style:text-properties fo:hyphenate=\"true\" fo:hyphenation-remain-char-count=\"2\" fo:hyphenation-push-char-count=\"2\"/>\n"
		"\t\t</style:style>\n"
		"\t\t<style:style style:name=\"First_Paragraph\" style:display-name=\"First Paragraph\" style:family=\"paragraph\" style:parent-style-name=\"Paragraph\">\n"
		"\t\t\t<style:paragraph-properties fo:margin-top=\"0.5cm\"/>\n"
		"\t\t</style:style>\n"
		"\t\t<style:style style:name=\"Indent_Paragraph\" style:display-name=\"Indent Paragraph\" style:family=\"paragraph\" style:parent-style-name=\"Paragraph\" style:class=\"text\">\n"
		"\t\t\t<style:paragraph-properties fo:text-indent=\"0.5cm\"/>\n"
		"\t\t</style:style>\n"
		"\t\t<style:style style:name=\"Blockquote\" style:family=\"paragraph\" style:parent-style-name=\"Paragraph\">\n"
		"\t\t\t<style:paragraph-properties fo:margin-left=\"1.25cm\" fo:margin-right=\"1.25cm\" fo:margin-top=\"0.5cm\" fo:keep-together=\"always\" fo:keep-with-next=\"always\"/>\n"
		"\t\t</style:style>\n"
		"\t\t<style:style style:name=\"Blockquote_Indent\" style:family=\"paragraph\" style:parent-style-name=\"Blockquote\">\n"
		"\t\t\t<style:paragraph-properties fo:margin-top=\"0.0cm\" fo:text-indent=\"0.5cm\"/>\n"
		"\t\t</style:style>\n"
		"\t\t<style:style style:name=\"Blockquote_Reference\" style:display-name=\"Blockquote Reference\" style:family=\"paragraph\" style:parent-style-name=\"Blockquote\">\n"
		"\t\t\t<style:paragraph-properties fo:margin-top=\"0.25cm\" fo:keep-with-next=\"auto\"/>\n"
		"\t\t</style:style>\n"
		"\t\t<style:style style:name=\"Emphasis\" style:family=\"text\">\n"
		"\t\t\t<style:text-properties fo:font-style=\"italic\"/>\n"
		"\t\t</style:style>\n"
		"\t\t<style:style style:name=\"Strong\" style:family=\"text\">\n"
		"\t\t\t<style:text-properties fo:font-weight=\"bold\"/>\n"
		"\t\t</style:style>\n"
		"\t</office:styles>\n"
		"\t<office:automatic-styles>\n"
		"\t\t<style:page-layout style:name=\"Letter\">\n"
		"\t\t\t<style:page-layout-properties fo:page-width=\"21.59cm\" fo:page-height=\"27.94cm\" style:num-format=\"1\" style:print-orientation=\"portrait\" fo:margin-top=\"2.54cm\" fo:margin-bottom=\"2.54cm\" fo:margin-left=\"3.81cm\" fo:margin-right=\"3.81cm\" style:writing-mode=\"lr-tb\" style:footnote-max-height=\"0cm\">\n"
		"\t\t\t\t<style:columns fo:column-count=\"1\" fo:column-gap=\"0cm\"/>\n"
		"\t\t\t\t<style:footnote-sep style:width=\"0.018cm\" style:distance-before-sep=\"0.101cm\" style:distance-after-sep=\"0.101cm\" style:line-style=\"solid\" style:adjustment=\"left\" style:rel-width=\"25%\" style:color=\"#000000\"/>\n"
		"\t\t\t</style:page-layout-properties>\n"
		"\t\t</style:page-layout>\n"
		"\t\t<style:page-layout style:name=\"Letter_Cover\">\n"
		"\t\t\t<style:page-layout-properties fo:page-width=\"21.59cm\" fo:page-height=\"27.94cm\" style:num-format=\"1\" style:print-orientation=\"portrait\" fo:margin-top=\"2.54cm\" fo:margin-bottom=\"2.54cm\" fo:margin-left=\"2cm\" fo:margin-right=\"2cm\" style:writing-mode=\"lr-tb\"/>\n"
		"\t\t</style:page-layout>\n"
		"\t</office:automatic-styles>\n"
		"\t<office:master-styles>\n"
		"\t\t<style:master-page style:name=\"Standard\" style:page-layout-name=\"Letter\"/>\n"
		"\t\t<style:master-page style:name=\"First_Page\" style:display-name=\"First Page\" style:page-layout-name=\"Letter_Cover\" style:next-style-name=\"Standard\"/>\n"
		"\t</office:master-styles>\n"
		"</office:document-styles>",
		f
	);

	fclose(f);
}

static void print_odt_text_block(FILE* f, const char* text)
{
	while (*text)
	{
		if (*text == text_token_type_strong_begin)
		{
			fputs("<text:span text:style-name=\"Strong\">", f);
		}
		else if (*text == text_token_type_strong_end)
		{
			fputs("</text:span>", f);
		}
		else if (*text == text_token_type_emphasis_begin)
		{
			fputs("<text:span text:style-name=\"Emphasis\">", f);
		}
		else if (*text == text_token_type_emphasis_end)
		{
			fputs("</text:span>", f);
		}
		else if (*text == text_token_type_reference)
		{
//			const uint32_t ref_count = ctx->inline_ref_count++ + 1;
//			const uint32_t chapter_ref_count = ctx->inline_chapter_ref_count++;
//
//			const document_chapter* chapter = &ctx->doc->chapters[ctx->chapter_index];
//			const document_reference* reference = &chapter->references[chapter_ref_count];
//
//			fprintf(ctx->f, "<sup><a id=\"ref-return%d\" href=\"#ref%d\" title=\"", ref_count, ref_count);
//			print_simple_text(ctx->f, reference->text);
//			fprintf(ctx->f, "\">[%d]</a></sup>", chapter_ref_count + 1);
		}
		else
		{
			print_char(f, *text);
		}

		++text;
	}
}

static void generate_odt_content(const document* doc)
{
	FILE* f = open_file(OUTPUT_DIR "/odt/content.xml", file_mode_write);

	html_context ctx = {
		.f		= f,
		.doc	= doc
	};

	fputs(
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<office:document-content xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\" office:version=\"1.3\">\n"
		"\t<office:body>\n"
		"\t\t<office:text>",
		f
	);

//	if (doc->metadata.type == document_type_book)
//	{
//		if (doc->metadata.title)
//			fprintf(f, "\n\t\t<h1 class=\"title\">%s</h1>", doc->metadata.title);
//
//		if (doc->metadata.author_count)
//		{
//			fprintf(f, "\n\t\t<p class=\"authors\">");
//
//			for (uint32_t i = 0; i < doc->metadata.author_count - 1; ++i)
//				fprintf(f, "\n\t\t\t%s<br>", doc->metadata.authors[i]);
//			fprintf(f, "\n\t\t\t%s", doc->metadata.authors[doc->metadata.author_count - 1]);
//
//			fprintf(f, "\n\t\t</p>");
//		}
//
//		if (doc->metadata.translator_count)
//		{
//			fprintf(f, "\n\t\t<p class=\"authors\">");
//			fprintf(f, "\n\t\t\tTranslated by:<br>");
//
//			for (uint32_t i = 0; i < doc->metadata.translator_count - 1; ++i)
//				fprintf(f, "\n\t\t\t%s<br>", doc->metadata.translators[i]);
//			fprintf(f, "\n\t\t\t%s", doc->metadata.translators[doc->metadata.translator_count - 1]);
//
//			fprintf(f, "\n\t\t</p>");
//		}
//
//		if (doc->chapter_count > 1)
//		{
//			fprintf(f,
//				"\n\t\t<h1>Contents</h1>\n"
//				"\t\t<p>\n"
//				"\t\t\t<ul class=\"chapters\">\n"
//			);
//
//			for (uint32_t chapter_index = 0; chapter_index < doc->chapter_count; ++chapter_index)
//			{
//				document_element* heading = doc->chapters[chapter_index].elements;
//
//				fprintf(f, "\t\t\t\t<li><a href=\"#h%d\">", chapter_index + 1);
//				print_simple_text(ctx.f, heading->text);
//				fprintf(f, "</a></li>\n");
//			}
//
//			fprintf(f,
//				"\t\t\t</ul>\n"
//				"\t\t</p>"
//			);
//		}
//	}

	int depth = 3;
	int paragraph_count = 0;
	bool inside_blockquote = false;

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
				paragraph_count = 0;
				ctx.chapter_ref_count = 0;
				ctx.inline_chapter_ref_count = 0;

				print_tabs(f, depth);
				fputs("<text:h text:style-name=\"Heading_1\" text:outline-level=\"1\">", f);
				print_odt_text_block(f, element->text);
				fprintf(f, "</text:h>");
				break;
			case document_element_type_heading_2:
				paragraph_count = 0;

				print_tabs(f, depth);
				fputs("<text:h text:style-name=\"Heading_2\" text:outline-level=\"2\">", f);
				print_odt_text_block(f, element->text);
				fprintf(f, "</text:h>");
				break;
			case document_element_type_heading_3:
				paragraph_count = 0;

				print_tabs(f, depth);
				fputs("<text:h text:style-name=\"Heading_3\" text:outline-level=\"3\">", f);
				print_odt_text_block(f, element->text);
				fprintf(f, "</text:h>");
				break;
			case document_element_type_text_block:
				print_odt_text_block(f, element->text);
				break;
			case document_element_type_line_break:
				fputs("<text:line-break/>", f);
				break;
			case document_element_type_paragraph_begin:
				++paragraph_count;

				print_tabs(f, depth);

				if (paragraph_count == 1)
				{
					if (inside_blockquote)
						fputs("<text:p text:style-name=\"Blockquote\">", f);
					else
						fputs("<text:p text:style-name=\"First_Paragraph\">", f);
				}
				else
				{
					if (inside_blockquote)
						fputs("<text:p text:style-name=\"Blockquote_Indent\">", f);
					else
						fputs("<text:p text:style-name=\"Indent_Paragraph\">", f);
				}

				break;
			case document_element_type_paragraph_break_begin:
				paragraph_count = 1;

				print_tabs(f, depth);

				if (inside_blockquote)
					fputs("<text:p text:style-name=\"Blockquote\">", f);
				else
					fputs("<text:p text:style-name=\"First_Paragraph\">", f);

				break;
			case document_element_type_paragraph_end:
				fputs("</text:p>", f);
				break;
			case document_element_type_blockquote_begin:
				paragraph_count = 0;
				inside_blockquote = true;
				break;
			case document_element_type_blockquote_end:
				paragraph_count = 0;
				inside_blockquote = false;
				break;
			case document_element_type_blockquote_citation:
				print_tabs(f, depth);
				fputs("<text:p text:style-name=\"Blockquote_Reference\">", f);
				print_em_dash(f);
				print_odt_text_block(f, element->text);
				fputs("</text:p>", f);
				break;
//			case document_element_type_ordered_list_begin_roman:
//				print_tabs(f, depth++);
//				fputs("<ol type=\"I\">", f);
//				break;
//			case document_element_type_ordered_list_begin_arabic:
//				print_tabs(f, depth++);
//				fputs("<ol>", f);
//				break;
//			case document_element_type_ordered_list_begin_letter:
//				print_tabs(f, depth++);
//				fputs("<ol type=\"a\">", f);
//				break;
//			case document_element_type_ordered_list_end:
//				print_tabs(f, --depth);
//				fputs("</ol>", f);
//				break;
//			case document_element_type_unordered_list_begin:
//				print_tabs(f, depth++);
//				fputs("<ul>", f);
//				break;
//			case document_element_type_unordered_list_end:
//				print_tabs(f, --depth);
//				fputs("</ul>", f);
//				break;
//			case document_element_type_list_item:
//				print_tabs(f, depth);
//				fprintf(f, "<li>%s</li>", element->text);
//				break;
			}
		}

//		if (chapter->reference_count > 0)
//		{
//			for (uint32_t reference_index = 0; reference_index < chapter->reference_count; ++reference_index)
//			{
//				++ctx.ref_count;
//				++ctx.chapter_ref_count;
//
//				document_reference* reference = &chapter->references[reference_index];
//				fprintf(f, "\n\t\t<p class=\"footnote\" id=\"ref%d\">\n", ctx.ref_count);
//				fprintf(f, "\t\t\t[<a href=\"#ref-return%d\">%d</a>] ", ctx.ref_count, ctx.chapter_ref_count);
//				print_html_text_block(&ctx, reference->text);
//				fprintf(f, "\n\t\t</p>");
//			}
//		}
	}

	fputs(
		"\n"
		"\t\t</office:text>\n"
		"\t</office:body>\n"
		"</office:document-content>",
		f
	);

	fclose(f);
}

static void generate_odt(const document* doc)
{
	create_dir(OUTPUT_DIR "\\odt\\META-INF");

	create_odt_mimetype();
	create_odt_meta_inf();
	create_odt_styles();
	generate_odt_content(doc);

	const char* inputs[] = {
		OUTPUT_DIR "/odt/mimetype",
		OUTPUT_DIR "/odt/META-INF/manifest.xml",
		OUTPUT_DIR "/odt/styles.xml",
		OUTPUT_DIR "/odt/content.xml"
	};
	const uint32_t input_count = sizeof(inputs) / sizeof(const char*);

	const char* outputs[] = {
		"mimetype",
		"META-INF/manifest.xml",
		"styles.xml",
		"content.xml"
	};
	static_assert(sizeof(outputs) == sizeof(inputs));
	const uint32_t output_count = sizeof(outputs) / sizeof(const char*);

	const char* odt_path = generate_path(OUTPUT_DIR "/%s.odt", doc->metadata.title);
	generate_zip(odt_path, inputs, outputs, output_count);

	delete_dir(OUTPUT_DIR "\\odt");
}