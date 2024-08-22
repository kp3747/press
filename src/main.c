static void print_usage(void)
{
	fprintf(stderr,
		"Usage:\n"
		"  press <src.txt> [--html|--epub]\n"
		"\n"
		"Flags:\n"
		"  none    validates source file and produces no output\n"
		"  --odt   generates ODT OpenDocument text file\n\n"
		"  --html  generates HTML webpage\n\n"
		"  --epub  generates ePub2 eBook\n\n"
	);

	exit(EXIT_FAILURE);
}

static void kpress_on_error(const char* str)
{
	MessageBoxA(nullptr, str, nullptr, MB_OK | MB_ICONERROR);
}

int main(int argc, const char** argv)
{
	bool odt = false;
	bool html = false;
	bool epub = false;
	char filepath_buffer[MAX_PATH];
	const char* filepath = nullptr;

	fputs("ARCP Press Tool v0.9.3\n", stdout);

	if (argc <= 1)
	{
		install_error_handler(kpress_on_error);

		// Open file dialog
		OPENFILENAME ofn = {
			.lStructSize	= sizeof(OPENFILENAME),
			//.hInstance		= instance,
			.lpstrFilter	= "Press-formatted text files (*.txt)\0*.txt\0\0",
			.lpstrFile		= filepath_buffer,
			.nMaxFile		= sizeof(filepath_buffer),
			.Flags			= OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR
		};
		if (!GetOpenFileNameA(&ofn))
			return 0;

		odt = html = epub = true;
		filepath = filepath_buffer;
	}
	else
	{
		for (int i = 1; i < argc; ++i)
		{
			if (*argv[i] == '-')
			{
				if (strcmp(argv[i], "--all") == 0)
					odt = html = epub = true;
				else if (strcmp(argv[i], "--odt") == 0)
					odt = true;
				else if (strcmp(argv[i], "--html") == 0)
					html = true;
				else if (strcmp(argv[i], "--epub") == 0)
					epub = true;
				else if (strcmp(argv[i], "--help") == 0)
					print_usage();
				else
					handle_error("Unsupported argument \"%s\".", argv[i]);
			}
			else
			{
				if (filepath)
					handle_error("Only a single source file is currently supported.");

				filepath = argv[i];
			}
		}

		if (!filepath)
			handle_error("No source file specified.");
	}

	char* text = load_file(filepath);

	document doc = {};

	line_tokens tokens;
	tokenise(text, &tokens, &doc.metadata);

	// Default to article to allow small documents without any metadata
	if (doc.metadata.type == document_type_none)
		doc.metadata.type = document_type_article;

	doc_mem_req mem_req;
	validate(&tokens, &mem_req);

	finalise(&tokens, &mem_req, &doc);

	if (!doc.metadata.title)
		doc.metadata.title = copy_filename(filepath);

	if (odt || html || epub)
	{
		delete_dir(output_dir);
		create_dir(output_dir);

		if (odt)
			generate_odt(&doc);
		if (html)
			generate_html(&doc);
		if (epub)
			generate_epub(&doc);

		printf("Generation successful\n");
	}
	else
	{
		printf("Validation successful\n");
	}

	if (argc <= 1)
		system("explorer press_output"); // Open output directory in file explorer

	return EXIT_SUCCESS;
}