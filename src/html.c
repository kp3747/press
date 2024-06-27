static void print_tabs(FILE* f, int depth)
{
	fputc('\n', f);

	for (int i = 0; i < depth; ++i)
		fputc('\t', f);
}

static void generate_html(const document* doc)
{
	const char* filepath = "out.html";

	FILE* f = fopen(filepath, "w");
	if (!f)
	{
		fprintf(stderr, "Error opening file for write '%s': %s.\n", filepath, strerror(errno));
		exit(EXIT_FAILURE);
	}

	fprintf(f,
		"<!DOCTYPE html>\n"
		"<html>\n"
		"\t<body>"
	);

	int depth = 2;

	for (uint32_t chapter_index = 0; chapter_index < doc->chapter_count; ++chapter_index)
	{
		document_chapter* chapter = &doc->chapters[chapter_index];

		for (uint32_t element_index = 0; element_index < chapter->element_count; ++element_index)
		{
			document_element* element = &chapter->elements[element_index];

			switch (element->type)
			{
			case document_element_type_heading_1:
				print_tabs(f, depth);
				fprintf(f, "<h1>%s</h1>", element->text);
				break;
			case document_element_type_heading_2:
				print_tabs(f, depth);
				fprintf(f, "<h2>%s</h2>", element->text);
				break;
			case document_element_type_heading_3:
				print_tabs(f, depth);
				fprintf(f, "<h3>%s</h3>", element->text);
				break;
			case document_element_type_text_block:
				print_tabs(f, depth + 1);
				fputs(element->text, f);
				break;
			case document_element_type_line_break:
				print_tabs(f, depth + 1);
				fputs("<br>", f);
				break;
			case document_element_type_paragraph_begin:
				print_tabs(f, depth);
				fputs("<p>", f);
				break;
			case document_element_type_paragraph_end:
				print_tabs(f, depth);
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
			}
		}
	}

	fprintf(f,
		"\n\t</body>\n"
		"</html>"
	);

	fclose(f);
}