typedef struct
{
	const char*	read_ptr;
	uint32_t	line;
	uint32_t	column;
	uint32_t	next_line;
	uint32_t	next_column;
	uint32_t	prev_line;
	uint32_t	prev_column;
	char		c;
	char		pc;
} peek_state;

typedef struct
{
	char*				buffer;
	char*				write_ptr;
	line_token*			current_line;
	line_token*			lines;
	uint32_t			line_count;
	uint32_t			line_capacity;
	uint32_t			ref_count;
	peek_state			peek;
	document_metadata*	metadata;
} tokenise_context;

static void handle_peek_error(const peek_state* peek, const char* format, ...);
static void handle_tokenise_error(const tokenise_context* ctx, const char* format, ...);
static line_token* add_line_token(tokenise_context* ctx, line_token_type type);
static void peek_init(tokenise_context* ctx, peek_state* peek);
static char peek_char(tokenise_context* ctx, peek_state* peek);
static void peek_apply(tokenise_context* ctx, peek_state* peek);
static char get_char(tokenise_context* ctx);