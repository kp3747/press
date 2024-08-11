/*
	TODO: Handle any filename length. Perhaps we could just add a global buffer of the maximum
	possible size and reuse between functions? For example 64 KiB * 4 = 256KiB. I believe Windows
	supports ~INT16_MAX for UTF-16. To be safe we could use UINT16_MAX * 4 (max UTF-8 length). For
	reference the max Linux file path is a more reasonable 4096. We could just support that and
	report an error if a path length exceeds that. In fact, that sounds like a good idea.
*/

static void create_dir(const char* dir)
{
	char buffer[256];
	const int len = snprintf(buffer, sizeof(buffer), "mkdir %s >nul 2>nul", dir);
	assert(len < sizeof(buffer));

	const int ret = system(buffer);
	if (ret)
		handle_error("Unable to create directory \"%s/\". Do you have a previously generated file open?", dir);
}

static void delete_dir(const char* dir)
{
	char buffer[256];
	const int len = snprintf(buffer, sizeof(buffer), "rmdir %s /s /q >nul 2>nul", dir);
	assert(len < sizeof(buffer));

	system(buffer);
}

static FILE* open_file(const char* path, file_mode mode)
{
	const char* mode_string;
	if (mode == file_mode_read)
		mode_string = "rb";
	else
		mode_string = "wb";

	FILE* f = fopen(path, mode_string);
	if (!f)
		handle_error("Unable to open file \"%s\": %s.\n", path, strerror(errno));

	return f;
}

static uint32_t get_file_size(FILE* f)
{
	// Get file size
	fseek(f, 0, SEEK_END);
	const long size = ftell(f);
	rewind(f);

	return size;
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

static void print_tabs(FILE* f, int depth)
{
	fputc('\n', f);
	for (int i = 0; i < depth; ++i)
		fputc('\t', f);
}

static void print_en_dash(FILE* f)
{
	fputc(0xE2, f);
	fputc(0x80, f);
	fputc(0x93, f);
}

static void print_em_dash(FILE* f)
{
	fputc(0xE2, f);
	fputc(0x80, f);
	fputc(0x94, f);
}

static void print_apostrophe(FILE* f)
{
	fputc(0xE2, f);
	fputc(0x80, f);
	fputc(0x99, f);
}

static void print_quote_level_1_begin(FILE* f)
{
	fputc(0xE2, f);
	fputc(0x80, f);
	fputc(0x9C, f);
}

static void print_quote_level_1_end(FILE* f)
{
	fputc(0xE2, f);
	fputc(0x80, f);
	fputc(0x9D, f);
}

static void print_char(FILE* f, char c)
{
	if (c == text_token_type_en_dash)
		print_en_dash(f);
	else if (c == text_token_type_em_dash)
		print_em_dash(f);
	else if (c == '\'')
		print_apostrophe(f);
	else if (c == text_token_type_quote_level_1_begin)
		print_quote_level_1_begin(f);
	else if (c == text_token_type_quote_level_1_end)
		print_quote_level_1_end(f);
	else if (c == text_token_type_left_square_bracket)
		fputc('[', f);
	else if (c == text_token_type_right_square_bracket)
		fputc(']', f);
	else
		fputc(c, f);
}

static void print_simple_text(FILE* f, const char* text)
{
	while (*text)
		print_char(f, *text++);
}