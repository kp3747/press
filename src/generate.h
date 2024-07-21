typedef struct
{
	FILE*			f;
	const document*	doc;
	int				ref_count;
	int				chapter_index;
	int				inline_ref_count;
	int				chapter_ref_count;
	int				inline_chapter_ref_count;
} html_context;

static void print_html_text_block(html_context* ctx, const char* text);

static void generate_odt(const document* doc);
static void generate_html(const document* doc);
static void generate_epub(const document* doc);