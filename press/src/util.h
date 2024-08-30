typedef enum
{
	file_mode_read,
	file_mode_write
} file_mode;

typedef struct
{
	void* f;
} file;

static char*		load_file(const char* filepath);
static void			create_dir(const char* dir);
static void			delete_dir(const char* dir);
static file			open_file(const char* path, file_mode mode);
static void			close_file(file f);
static uint32_t		get_file_size(file f);
static void			read_file(file f, void* dst, uint32_t size);
static void			write_file(file f, const void* src, size_t size);
static const char*	copy_filename(const char* filepath);
static const char*	generate_path(const char* format, ...);

static void			print_str(file f, const char* s);
static void			print_fmt(file f, const char* format, ...);
static void			print_char(file f, char c);
static void			print_char_token(file f, char c);
static void			print_tabs(file f, int depth);
static void			print_en_dash(file f);
static void			print_em_dash(file f);
static void			print_apostrophe(file f);
static void			print_quote_level_1_begin(file f);
static void			print_quote_level_1_end(file f);
static void			print_quote_level_2_begin(file f);
static void			print_quote_level_2_end(file f);
static void			print_simple_text(file f, const char* text);

// TODO: Move these to error.h
typedef void (*error_callback)(const char* str);

static void			install_error_handler(error_callback handler);
static void			handle_error(const char* format, ...);
static void			exit_failure(void);
static void			print_error(const char* format, ...);
static void			print_error_char(char c);
static void			print_error_vargs(const char* format, va_list args);