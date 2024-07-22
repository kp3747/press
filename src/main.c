static char* load_file(const char* filepath)
{
	FILE* f = open_file(filepath, file_mode_read);
	const uint32_t size = get_file_size(f);

	/*
		Allocate enough memory plus two bytes:
		1. Potential extra new line character before null terminator to make parsing simpler.
		2. Null terminator.
	*/
	char* data = malloc(size + 2);

	// Put data one byte past the beginning of the buffer to allow space for initial control code
	fread(data, 1, size, f);
	fclose(f);

	// Check if final new line character needs to be added, then null terminate
	if (data[size - 1] == '\n')
	{
		data[size] = 0;
	}
	else
	{
		data[size] = '\n';
		data[size + 1] = 0;
	}

	return data;
}

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

static const char* copy_filename(const char* filepath)
{
	assert(filepath);
	assert(*filepath);

	const char* last_dir = filepath;
	const char* last_dot = nullptr;

	for (;;)
	{
		const char c = *filepath;
		if (c == '\\' || c == '/')
			last_dir = filepath + 1;
		else if (c == '.')
			last_dot = filepath;
		else if (c == 0)
			break;

		++filepath;
	}

	if (!last_dot)
		last_dot = filepath;

	const int64_t len = last_dot - last_dir;
	assert(len);

	char* filename = malloc(len + 1);
	for (int64_t i = 0; i < len; ++i)
		filename[i] = last_dir[i];
	filename[len] = 0;

	return filename;
}

static void delete_dir(const char* dir)
{
	char buffer[256];
	const int len = snprintf(buffer, sizeof(buffer), "rmdir %s /s /q >nul 2>nul", dir);
	assert(len < sizeof(buffer));

	system(buffer);
}

int main(int argc, const char** argv)
{
	bool odt = false;
	bool html = false;
	bool epub = false;
	const char* filepath = nullptr;

	fputs("ARCP Press Tool v0.9.0\n", stdout);

	if (argc <= 1)
		print_usage();

	for (int i = 1; i < argc; ++i)
	{
		if (*argv[i] == '-')
		{
			if (strcmp(argv[i], "--odt") == 0)
				odt = true;
			else if (strcmp(argv[i], "--html") == 0)
				html = true;
			else if (strcmp(argv[i], "--epub") == 0)
				epub = true;
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

	return EXIT_SUCCESS;
}