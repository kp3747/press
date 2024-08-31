typedef enum
{
	line_token_type_none,
	line_token_type_eof,
	line_token_type_note,
	line_token_type_dinkus,
	line_token_type_newline,
	line_token_type_paragraph,
	line_token_type_heading_1,
	line_token_type_heading_2,
	line_token_type_heading_3,
	line_token_type_preformatted,
	line_token_type_right_aligned,
	line_token_type_block_newline,
	line_token_type_unordered_list,
	line_token_type_block_citation,
	line_token_type_block_paragraph,
	line_token_type_paragraph_break,
	line_token_type_ordered_list_roman,
	line_token_type_ordered_list_arabic,
	line_token_type_ordered_list_letter
} line_token_type;

typedef struct
{
	line_token_type	type;
	uint32_t		line;
	char*			text;
	uint32_t		index;
	uint32_t		length;
	uint32_t		padding[2];
} line_token;
static_assert(sizeof(line_token) == 32);								// Prevent accidental change
static_assert((sizeof(line_token) & (sizeof(line_token) - 1)) == 0);	// Ensure power of two

typedef struct
{
	line_token*	lines;
	uint32_t	count;
} line_tokens;

static void tokenise(char* data, line_tokens* out_tokens, document_metadata* metadata);