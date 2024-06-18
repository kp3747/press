typedef enum
{
	line_token_type_none,
	line_token_type_eof,
	line_token_type_newline,
	line_token_type_metadata,
	line_token_type_paragraph,
	line_token_type_heading_1,
	line_token_type_heading_2,
	line_token_type_heading_3,
	line_token_type_reference,
	line_token_type_preformatted,
	line_token_type_ordered_list,
	line_token_type_unordered_list,
	line_token_type_block_nmewline,
	line_token_type_block_paragraph,
	line_token_type_block_ordered_list,
	line_token_type_block_unordered_list
} line_token_type;

typedef enum
{
	text_token_type_quote,
	text_token_type_strong,
	text_token_type_en_dash,
	text_token_type_em_dash,
	text_token_type_emphasis,
	text_token_type_reference,
	text_token_type_apostrophe,
	text_token_type_preformatted
} text_token_type;

typedef struct
{
	uint32_t offset;
	uint32_t length	: 29;
	uint32_t type	: 3;	// text_token_type
} line_token;

static_assert(sizeof(line_token) == 8);									// Prevent accidental change
static_assert((sizeof(line_token) & (sizeof(line_token) - 1)) == 0);	// Ensure power of two

typedef struct
{
	char*		buffer;
	const char*	read_ptr;
	char*		write_ptr;
	line_token*	lines;
	uint32_t	line_count;
	uint32_t	line_capacity;
	uint32_t	line;
	uint32_t	column;
	uint32_t	next_line;
	uint32_t	next_column;
	char		c;
	char		pc;
} tokenise_context;

static char* load_file(const char* filepath)
{
	FILE* f = fopen(filepath, "rb");
	if (!f)
		fprintf(stderr, "Error opening file '%s': %s.\n", filepath, strerror(errno)); 

	// Get file size
	fseek(f, 0, SEEK_END);
	const long size = ftell(f);
	rewind(f);

	/*
		Allocate enough memory plus two bytes:
		1. Potential extra new line character before null terminator to make parsing simpler.
		2. Null terminator.
	*/
	char* data = malloc(size + 2);

	// Put data one byte past the beginning of the buffer to allow space for initial control code
	fread(data, 1, size, f);

	// Check if final new line character needs to be added, then null terminate
	if (data[size - 1] == '\n')
	{
		data[size] = 0;
	}
	else
	{
		data[size] = '\n';
		data[size + 1] = 0;
	}

	return data;
}

static line_token* add_line_token(tokenise_context* ctx, line_token_type type)
{
	if (ctx->line_count != ctx->line_capacity)
		return &ctx->lines[ctx->line_count++];

	const uint32_t elements_per_page = page_size / sizeof(line_token);
	const uint32_t current_size = ctx->line_capacity ? ctx->line_capacity / elements_per_page : page_size;
	const uint32_t next_size = current_size + page_size;

	ctx->lines = realloc(ctx->lines, next_size);
	ctx->line_capacity += elements_per_page;

	return &ctx->lines[ctx->line_count++];
}