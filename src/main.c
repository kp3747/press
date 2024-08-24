static char filepath_buffer[64 << 10];

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

	fputs("ARCP Press Tool v0.9.4\n", stdout);

	mem_init();
	void* frame = mem_push();

	const char** filepaths = frame;
	uint32_t filepath_count = 0;

	if (argc <= 1)
	{
		install_error_handler(kpress_on_error);

		odt = html = epub = true;

		// Open file dialog
		OPENFILENAMEA ofn = {
			.lStructSize	= sizeof(OPENFILENAME),
			.lpstrFilter	= "Press-formatted text files (*.txt)\0*.txt\0\0",
			.lpstrFile		= filepath_buffer,
			.nMaxFile		= sizeof(filepath_buffer),
			.Flags			= OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR
		};
		if (!GetOpenFileNameA(&ofn))
			return 0;

		/*
			GetOpenFileNameA() in multi-select mode returns the following file string format:
			* Directory path + null terminator.
			* Multiple filenames and extensions + null terminator as separator.
			* Final null list terminator.
		*/
		
		// Cache directory string and length
		const char* dir = filepath_buffer;
		const int64_t dir_len = strlen(dir);

		// Count number of file paths
		const char* current = filepath_buffer + dir_len + 1;
		for (;;)
		{
			if (*current++ == 0)
			{
				filepath_count++;
				if (*current == 0)
					break;
			}
		}

		// Allocate file path array
		filepaths = mem_alloc(sizeof(const char*) * filepath_count);
		filepath_count = 0;

		// Allocate and fill file paths
		current = filepath_buffer + dir_len + 1;
		do
		{
			const int64_t file_len = strlen(current);
			char* path = mem_alloc(dir_len + 1 + file_len + 1);
			memcpy(path, dir, dir_len);
			path[dir_len] = '\\';
			memcpy(path + dir_len + 1, current, file_len + 1);
			current += file_len + 1;

			filepaths[filepath_count++] = path;
		} while(*current != 0);
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
				mem_alloc(sizeof(const char*));
				filepaths[filepath_count++] = argv[i];
			}
		}
	}

	if (!filepath_count)
		handle_error("No source file specified.");

	const bool generate = odt || html || epub;
	if (generate)
	{
		delete_dir(output_dir);
		create_dir(output_dir);
	}

	for (uint32_t i = 0; i < filepath_count; ++i)
	{
		void* frame = mem_push();

		printf("Processing \"%s\"\n", filepaths[i]);

		char* text = load_file(filepaths[i]);

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
			doc.metadata.title = copy_filename(filepaths[i]);

		if (generate)
		{
			if (odt)
				generate_odt(&doc);
			if (html)
				generate_html(&doc);
			if (epub)
				generate_epub(&doc);
		}

		mem_pop(frame);
	}

	mem_pop(frame);
	mem_term();

	if (generate)
		printf("Generation successful\n");
	else
		printf("Validation successful\n");

	if (argc <= 1)
		system("explorer press_output"); // Open output directory in file explorer

	return EXIT_SUCCESS;
}