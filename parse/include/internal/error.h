typedef void (*error_callback)(const char* str);

void install_error_handler(error_callback handler);
void handle_error(const char* format, ...);
void exit_failure(void);
void print_error(const char* format, ...);
void print_error_char(char c);
void print_error_vargs(const char* format, va_list args);