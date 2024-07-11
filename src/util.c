static FILE* open_file(const char* path, file_mode mode)
{
	const char* mode_string;
	if (mode == file_mode_read)
		mode_string = "rb";
	else
		mode_string = "w";

	FILE* f = fopen(path, mode_string);
	if (!f)
		handle_error("Error opening file \"%s\": %s.\n", path, strerror(errno));

	return f;
}

static void handle_error(const char* format, ...)
{
	fputs("Error: ", stderr);

	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);

	fputc('\n', stderr);

	assert(false);
	exit(EXIT_FAILURE);
}