typedef struct
{
	FILE*			f;
	const document*	doc;
	int				depth;
	int				ref_count;
	int				chapter_index;
	int				inline_ref_count;
	int				chapter_ref_count;
	int				inline_chapter_ref_count;
} html_context;

static void print_tabs(html_context* ctx, int depth);
static void print_text_simple(html_context* ctx, const char* text);
static void print_text_block(html_context* ctx, const char* text);

static void generate_html(const document* doc);
static void generate_epub(const document* doc);