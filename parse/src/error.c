static char				error_buffer[4096];
static uint32_t			error_buffer_pos;
static error_callback	error_handler;

void install_error_handler(error_callback handler)
{
	error_handler = handler;
}

void handle_error(const char* format, ...)
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

void exit_failure(void)
{
	fputs(error_buffer, stderr);

	// Break in debugger first
	debug_trap();

	// Show error message box in GUI mode
	if (error_handler)
		error_handler(error_buffer);

	exit(EXIT_FAILURE);
}

void print_error(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	print_error_vargs(format, args);
	va_end(args);
}

void print_error_char(char c)
{
	if (error_buffer_pos < sizeof(error_buffer) - 1)
		error_buffer[error_buffer_pos++] = c;
}

void print_error_vargs(const char* format, va_list args)
{
	const int len = vsnprintf(error_buffer + error_buffer_pos, sizeof(error_buffer) - error_buffer_pos, format, args);
	error_buffer_pos += len;
	if (error_buffer_pos == sizeof(error_buffer))
		error_buffer_pos = sizeof(error_buffer);
}