static char* load_file(const char* filepath)
{
	FILE* f = open_file(filepath, file_mode_read);

	// Get file size
	fseek(f, 0, SEEK_END);
	const long size = ftell(f);
	rewind(f);

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
		"  press <src.txt> [--html]\n"
		"\n"
		"Flags:\n"
		"  none    validates source file and produces no output\n"
		"  --html  generates HTML output\n\n"
		"  --epub  generates ePub2 output\n\n"
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
	const int len = snprintf(buffer, sizeof(buffer), "rmdir %s /s /q", dir);
	assert(len < sizeof(buffer));

	const int ret = system(buffer);
	if (ret)
		handle_error("Unable to delete directory \"%s\"", dir);
}

static void create_dir(const char* dir)
{
	char buffer[256];
	const int len = snprintf(buffer, sizeof(buffer), "mkdir %s", dir);
	assert(len < sizeof(buffer));

	const int ret = system(buffer);
	if (ret)
		handle_error("Unable to create directory \"%s\"", dir);
}

int main(int argc, const char** argv)
{
	if (argc <= 1)
		print_usage();

	const char* filepath = argv[1];

	char* text = load_file(filepath);

	line_tokens tokens;
	tokenise(text, &tokens);

	doc_mem_req mem_req;
	validate(&tokens, &mem_req);

	document doc;
	finalise(&tokens, &mem_req, &doc);

	if (!doc.metadata.title)
		doc.metadata.title = copy_filename(filepath);

	delete_dir("press_output");
	create_dir("press_output");

	generate_html(&doc);
	generate_epub(&doc);

	return EXIT_SUCCESS;
}