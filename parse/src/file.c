/*
	TODO: Handle any filename length. Perhaps we could just add a global buffer of the maximum
	possible size and reuse between functions? For example 64 KiB * 4 = 256KiB. I believe Windows
	supports ~INT16_MAX for UTF-16. To be safe we could use UINT16_MAX * 4 (max UTF-8 length). For
	reference the max Linux file path is a more reasonable 4096. We could just support that and
	report an error if a path length exceeds that. In fact, that sounds like a good idea.
*/

static char* load_file(const char* filepath)
{
	file f = open_file(filepath, file_mode_read);
	const uint32_t size = get_file_size(f);

	/*
		Allocate enough memory plus two bytes:
		1. Potential extra new line character before null terminator to make parsing simpler.
		2. Null terminator.
	*/
	char* data = mem_alloc(size + 2);

	// Put data one byte past the beginning of the buffer to allow space for initial control code
	fread(data, 1, size, f.f);
	fclose(f.f);

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

//static void create_dir(const char* dir)
//{
//	const BOOL result = CreateDirectoryA(dir, nullptr);
//	if (!result)
//		handle_error("Unable to create directory \"%s\". Do you have a previously generated file open?", dir);
//}

//static void delete_dir(const char* dir)
//{
//	void* frame = mem_push();
//
//	const int64_t len = strlen(dir);
//	char* file_path = mem_alloc(len + 2);
//	memcpy(file_path, dir, len);
//
//	// The API requires path to end with two null terminators
//	file_path[len] = file_path[len + 1] = 0;
//
//	SHFILEOPSTRUCT op = {
//		.wFunc	= FO_DELETE,
//		.pFrom	= file_path,
//		.fFlags	= FOF_NO_UI
//	};
//
//	SHFileOperationA(&op);
//
//	mem_pop(frame);
//}

static file open_file(const char* path, file_mode mode)
{
	const char* mode_string;
	if (mode == file_mode_read)
		mode_string = "rb";
	else
		mode_string = "wb";

	file f;
	f.f = fopen(path, mode_string);
	if (!f.f)
		handle_error("Unable to open file \"%s\": %s.\n", path, strerror(errno));

	return f;
}

static void close_file(file f)
{
	fclose(f.f);
}

static uint32_t get_file_size(file f)
{
	// Get file size
	fseek(f.f, 0, SEEK_END);
	const long size = ftell(f.f);
	rewind(f.f);

	return size;
}

static void read_file(file f, void* dst, uint32_t size)
{
	fread(dst, 1, size, f.f);
}

static void write_file(file f, const void* src, size_t size)
{
	fwrite(src, 1, size, f.f);
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

	char* filename = mem_alloc(len + 1);
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
	char* path = mem_alloc(len + 1);

	vsnprintf(path, len + 1, format, args);
	va_end(args);

	return path;
}