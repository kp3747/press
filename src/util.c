static FILE* open_file(const char* path, file_mode mode)
{
	const char* mode_string;
	if (mode == file_mode_read)
		mode_string = "rb";
	else
		mode_string = "w";

	FILE* f = fopen(path, mode_string);
	if (!f)
	{
		fprintf(stderr, "Error opening file \"%s\": %s.\n", path, strerror(errno));
		assert(false);
		exit(EXIT_FAILURE);
	}

	return f;
}