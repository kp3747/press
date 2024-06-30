static void print_tabs(FILE* f, int depth)
{
	fputc('\n', f);

	for (int i = 0; i < depth; ++i)
		fputc('\t', f);
}

static void print_text_block(FILE* f, const char* text)
{
	while (*text)
	{
		if (*text == text_token_type_strong_begin)
		{
			fputs("<strong>", f);
		}
		else if (*text == text_token_type_strong_end)
		{
			fputs("</strong>", f);
		}
		if (*text == text_token_type_emphasis_begin)
		{
			fputs("<em>", f);
		}
		else if (*text == text_token_type_emphasis_end)
		{
			fputs("</em>", f);
		}
		else if (*text == text_token_type_en_dash)
		{
			fputc(0xE2, f);
			fputc(0x80, f);
			fputc(0x93, f);
		}
		else if (*text == text_token_type_em_dash)
		{
			fputc(0xE2, f);
			fputc(0x80, f);
			fputc(0x94, f);
		}
		else if (*text == '\'')
		{
			fputc(0xE2, f);
			fputc(0x80, f);
			fputc(0x99, f);
		}
		else if (*text == text_token_type_quote_level_1_begin)
		{
			fputc(0xE2, f);
			fputc(0x80, f);
			fputc(0x9C, f);
		}
		else if (*text == text_token_type_quote_level_1_end)
		{
			fputc(0xE2, f);
			fputc(0x80, f);
			fputc(0x9D, f);
		}
		else
		{
			fputc(*text, f);
		}

		++text;
	}
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
		"<html lang=\"en-GB\">\n"
		"\t<head>\n"
		"\t\t<meta charset=\"UTF-8\">\n"
		"\t\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
		"\t\t<link href=\"style.css\" rel=\"stylesheet\">\n"
		"\t</head>\n"
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
				print_text_block(f, element->text);
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
			case document_element_type_ordered_list_item:
				print_tabs(f, depth);
				fprintf(f, "<li>%s</li>", element->text);
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