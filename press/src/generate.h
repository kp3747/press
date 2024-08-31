#define OUTPUT_DIR "press_output"
static const char output_dir[] = OUTPUT_DIR;
enum
{
	output_len = sizeof(output_dir) - 1
};

typedef struct
{
	file			f;
	const document*	doc;
	int				note_count;
	int				chapter_index;
	int				inline_note_count;
	int				chapter_note_count;
	int				inline_chapter_note_count;
} html_context;

static void print_html_text_block(html_context* ctx, const char* text);
static void generate_zip(const char* filepath, const char** input_files, const char** output_files, uint32_t count);

static void generate_odt(const document* doc);
static void generate_html(const document* doc);
static void generate_epub(const document* doc, const char* cover);