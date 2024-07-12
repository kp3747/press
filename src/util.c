static void create_dir(const char* dir)
{
	char buffer[256];
	const int len = snprintf(buffer, sizeof(buffer), "mkdir %s", dir);
	assert(len < sizeof(buffer));

	const int ret = system(buffer);
	if (ret)
		handle_error("Unable to create directory \"%s/\"", dir);
}

static void push_dir(const char* dir)
{
	char buffer[256];
	const int len = snprintf(buffer, sizeof(buffer), "cd %s", dir);
	assert(len < sizeof(buffer));

	const int ret = system(buffer);
	if (ret)
		handle_error("Unable to open directory \"%s/\"", dir);
}

static void pop_dir(void)
{
	const int ret = system("cd ..");
	assert(ret);
}

static FILE* open_file(const char* path, file_mode mode)
{
	const char* mode_string;
	if (mode == file_mode_read)
		mode_string = "rb";
	else
		mode_string = "w";

	FILE* f = fopen(path, mode_string);
	if (!f)
		handle_error("Unable to open file \"%s\": %s.\n", path, strerror(errno));

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

static const char* generate_path(const char* format, ...)
{
	va_list args;
	va_start(args, format);

	const int len = vsnprintf(nullptr, 0, format, args);
	char* path = malloc(len + 1);

	vsnprintf(path, len + 1, format, args);
	va_end(args);

	return path;
}