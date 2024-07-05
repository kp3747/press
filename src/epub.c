static void create_epub_mimetype(void)
{
	FILE* f = open_file("ebook_out/mimetype", file_mode_write);

	fputs("application/epub+zip", f);

	fclose(f);
}

static void create_epub_meta_inf(void)
{
	FILE* f = open_file("ebook_out/META-INF/container.xml", file_mode_write);

	fputs(
		"<?xml version=\"1.0\"?>\n"
		"<container version=\"1.0\" xmlns=\"urn:oasis:names:tc:opendocument:xmlns:container\">\n"
		"\t<rootfiles>\n"
		"\t\t<rootfile full-path=\"content.opf\" media-type=\"application/oebps-package+xml\" />\n"
		"\t</rootfiles>\n"
		"</container>",
		f
	);

	fclose(f);
}

static void create_epub_opf(const document* doc)
{
	FILE* f = open_file("ebook_out/content.opf", file_mode_write);

	fputs(
		"<?xml version=\"1.0\"?>\n"
		"<package version=\"2.0\" xmlns=\"http://www.idpf.org/2007/opf\" unique-identifier=\"bookid\">\n"
		"\t<metadata xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:opf=\"http://www.idpf.org/2007/opf\">\n",
		f
	);

	fprintf(f, "\t\t<dc:title>%s</dc:title>\n", doc->metadata.title);
	fprintf(f, "\t\t<dc:language>en-GB</dc:language>\n");
	fprintf(f, "\t\t<dc:identifier id=\"bookid\" opf:scheme=\"uuid\">6519aff0-f47c-437c-ba20-cc0782b39b05</dc:identifier>\n");

	for (uint32_t i = 0; i < doc->metadata.author_count; ++i)
		fprintf(f, "\t\t<dc:creator opf:role=\"author\">%s</dc:creator>\n", doc->metadata.authors[i]);

	for (uint32_t i = 0; i < doc->metadata.translator_count; ++i)
		fprintf(f, "\t\t<dc:creator opf:role=\"translator\">%s</dc:creator>\n", doc->metadata.translators[i]);

	fputs("\t<manifest>\n", f);
	fputs("\t\t<item id=\"ncx\" href=\"toc.ncx\" media-type=\"application/x-dtbncx+xml\">\n", f);

	for (uint32_t i = 0; i < doc->chapter_count; ++i)
		fprintf(f, "\t\t<item id=\"chapter%d\" href=\"chapter%d.xhtml\" media-type=\"application/xhtml+xml\"/>\n", i + 1, i + 1);

	fputs("\t</manifest>\n", f);

	fputs("\t<spine toc=\"ncx\">\n", f);

	for (uint32_t i = 0; i < doc->chapter_count; ++i)
		fprintf(f, "\t\t<itemref idref=\"chapter%d\"/>\n", i + 1);

	fputs("\t</spine>\n", f);

	fputs("</package>", f);

	fclose(f);
}

static void create_epub_ncx(const document* doc)
{
	FILE* f = open_file("ebook_out/toc.ncx", file_mode_write);

	fputs(
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
		"<ncx xmlns=\"http://www.daisy.org/z3986/2005/ncx/\" version=\"2005-1\">\n"
		"\t<head>\n"
		"\t\t<meta name=\"dtb:uid\" content=\"6519aff0-f47c-437c-ba20-cc0782b39b05\"/>\n"
//		"\t\t<meta name=\"dtb:depth\" content=\"1\"/>\n"
//		"\t\t<meta name=\"dtb:totalPageCount\" content=\"0\"/>\n"
//		"\t\t<meta name=\"dtb:maxPageNumber\" content=\"0\"/>\n"
		"\t</head>\n",
		f
	);

//	fputs("\t<docTitle>", f);
//	fprintf(f, "\t\t<text>%s</text>\n", doc->metadata.title);
//	fputs("\t</docTitle>", f);

	fputs("\t<navMap>\n", f);

	for (uint32_t i = 0; i < doc->chapter_count; ++i)
	{
		document_element* heading = doc->chapters[i].elements;

		fprintf(f, "\t\t<navPoint class=\"chapter\" id=\"chapter%d\" playOrder=\"%d\">\n", i + 1, i + 1);
		fputs("\t\t\t<navLabel>\n", f);
		fprintf(f, "\t\t\t\t<text>%s</text>\n", heading->text);
		fputs("\t\t\t</navLabel>\n", f);
		fprintf(f, "\t\t\t<content src=\"chapter%d.xhtml\"/>\n", i + 1);
		fputs("\t\t<navPoint>\n", f);
	}

	fputs("\t</navMap>\n", f);
	fputs("</ncx>", f);

	fclose(f);
}

static void generate_epub(const document* doc)
{
	create_epub_mimetype();
	create_epub_meta_inf();
	create_epub_opf(doc);
	create_epub_ncx(doc);
}