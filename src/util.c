//static void handle_parse_error(const char* label, uint32_t line, uint32_t column, const char* format, ...)
//{
//	fprintf(stderr, "%s (line %u, column %u): ", line, column);
//
//	va_list args;
//	va_start(args, format);
//	vfprintf(stderr, format, args);
//	va_end(args);
//
//	fputc('\n', stderr);
//
//	exit(EXIT_FAILURE);
//}