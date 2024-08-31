file open_file(const char* path, file_mode mode)
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

void close_file(file f)
{
	fclose(f.f);
}

uint32_t get_file_size(file f)
{
	// Get file size
	fseek(f.f, 0, SEEK_END);
	const long size = ftell(f.f);
	rewind(f.f);

	return size;
}

void read_file(file f, void* dst, uint32_t size)
{
	fread(dst, 1, size, f.f);
}

void write_file(file f, const void* src, size_t size)
{
	fwrite(src, 1, size, f.f);
}

const char* copy_filename(const char* filepath)
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

const char* generate_path(const char* format, ...)
{
	va_list args;
	va_start(args, format);

	const int len = vsnprintf(nullptr, 0, format, args);
	char* path = mem_alloc(len + 1);

	vsnprintf(path, len + 1, format, args);
	va_end(args);

	return path;
}