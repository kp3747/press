/*
	TODO: Handle any filename length. Perhaps we could just add a global buffer of the maximum
	possible size and reuse between functions? For example 64 KiB * 4 = 256KiB. I believe Windows
	supports ~INT16_MAX for UTF-16. To be safe we could use UINT16_MAX * 4 (max UTF-8 length). For
	reference the max Linux file path is a more reasonable 4096. We could just support that and
	report an error if a path length exceeds that. In fact, that sounds like a good idea.
*/

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

static void create_dir(const char* dir)
{
	const BOOL result = CreateDirectoryA(dir, nullptr);
	if (!result)
		handle_error("Unable to create directory \"%s\". Do you have a previously generated file open?", dir);
}

static void delete_dir(const char* dir)
{
	const int64_t len = strlen(dir);
	char* file_path = malloc(len + 2);
	memcpy(file_path, dir, len);

	// The API requires path to end with two null terminators
	file_path[len] = file_path[len + 1] = 0;

	SHFILEOPSTRUCT op = {
		.wFunc	= FO_DELETE,
		.pFrom	= file_path,
		.fFlags	= FOF_NO_UI
	};

	SHFileOperationA(&op);
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

static void print_quote_level_2_begin(FILE* f)
{
	fputc(0xE2, f);
	fputc(0x80, f);
	fputc(0x98, f);
}

static void print_quote_level_2_end(FILE* f)
{
	fputc(0xE2, f);
	fputc(0x80, f);
	fputc(0x99, f);
}

static void print_char(FILE* f, char c)
{
	switch (c)
	{
	case text_token_type_en_dash:
		print_en_dash(f);
		break;
	case text_token_type_em_dash:
		print_em_dash(f);
		break;
	case '\'':
		print_apostrophe(f);
		break;
	case text_token_type_quote_level_1_begin:
		print_quote_level_1_begin(f);
		break;
	case text_token_type_quote_level_1_end:
		print_quote_level_1_end(f);
		break;
	case text_token_type_quote_level_2_begin:
		print_quote_level_2_begin(f);
		break;
	case text_token_type_quote_level_2_end:
		print_quote_level_2_end(f);
		break;
	case text_token_type_left_square_bracket:
		fputc('[', f);
		break;
	case text_token_type_right_square_bracket:
		fputc(']', f);
		break;
	case text_token_type_reference:
	case text_token_type_strong_end:
	case text_token_type_emphasis_end:
	case text_token_type_strong_begin:
	case text_token_type_emphasis_begin:
		break;
	default:
		fputc(c, f);
	}
}

static void print_simple_text(FILE* f, const char* text)
{
	while (*text)
		print_char(f, *text++);
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

static char				error_buffer[4096];
static uint32_t			error_buffer_pos;
static error_callback	error_handler;

static void install_error_handler(error_callback handler)
{
	error_handler = handler;
}

static void exit_failure(void)
{
	fputs(error_buffer, stderr);

	// Break in debugger first
	debug_trap();

	// Show error message box in GUI mode
	if (error_handler)
		error_handler(error_buffer);

	exit(EXIT_FAILURE);
}

static void print_error(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	print_error_vargs(format, args);
	va_end(args);
}

static void print_error_char(char c)
{
	if (error_buffer_pos < sizeof(error_buffer) - 1)
		error_buffer[error_buffer_pos++] = c;
}

static void print_error_vargs(const char* format, va_list args)
{
	const int len = vsnprintf(error_buffer + error_buffer_pos, sizeof(error_buffer) - error_buffer_pos, format, args);
	error_buffer_pos += len;
	if (error_buffer_pos == sizeof(error_buffer))
		error_buffer_pos = sizeof(error_buffer);
}