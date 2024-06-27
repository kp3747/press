static char* load_file(const char* filepath)
{
	FILE* f = fopen(filepath, "rb");
	if (!f)
	{
		fprintf(stderr, "Error opening file '%s': %s.\n", filepath, strerror(errno));
		exit(EXIT_FAILURE);
	}

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
	);

	exit(EXIT_FAILURE);
}

int main(int argc, const char** argv)
{
	if (argc <= 1)
		print_usage();

	char* text = load_file(argv[1]);

	line_tokens tokens;
	tokenise(text, &tokens);

	doc_mem_req mem_req;
	validate(&tokens, &mem_req);

	document doc;
	finalise(&tokens, &mem_req, &doc);

	generate_html(&doc);

	return EXIT_SUCCESS;
}