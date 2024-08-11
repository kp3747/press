typedef enum
{
	file_mode_read,
	file_mode_write
} file_mode;

static void			create_dir(const char* dir);
static void			delete_dir(const char* dir);
static FILE*		open_file(const char* path, file_mode mode);
static uint32_t		get_file_size(FILE* f);
static void			handle_error(const char* format, ...);
static const char*	generate_path(const char* format, ...);
static void			print_tabs(FILE* f, int depth);
static void			print_en_dash(FILE* f);
static void			print_em_dash(FILE* f);
static void			print_apostrophe(FILE* f);
static void			print_quote_level_1_begin(FILE* f);
static void			print_quote_level_1_end(FILE* f);
static void			print_char(FILE* f, char c);
static void			print_simple_text(FILE* f, const char* text);