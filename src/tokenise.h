typedef enum
{
	line_token_type_none,
	line_token_type_eof,
	line_token_type_newline,
	line_token_type_paragraph,
	line_token_type_heading_1,
	line_token_type_heading_2,
	line_token_type_heading_3,
	line_token_type_reference,
	line_token_type_preformatted,
	line_token_type_block_newline,
	line_token_type_unordered_list,
	line_token_type_block_citation,
	line_token_type_block_paragraph,
	line_token_type_paragraph_break,
	line_token_type_ordered_list_roman,
	line_token_type_ordered_list_arabic,
	line_token_type_ordered_list_letter
} line_token_type;

/*
	These values must remain below 32 (space), and suported ASCII characters must line up with ASCII
	value. Currently these are null, tab, and newline.
*/
typedef enum
{
	text_token_type_null					= 0,
	text_token_type_en_dash					= 1,
	text_token_type_em_dash					= 2,
	text_token_type_reference				= 3,
	text_token_type_strong_end				= 4,
	text_token_type_emphasis_end			= 5,
	text_token_type_preformatted			= 6,
	text_token_type_strong_begin			= 7,
	text_token_type_emphasis_begin			= 8,
	text_token_type_tab						= 9,
	text_token_type_newline					= 10,
	text_token_type_quote_level_1_begin		= 11,
	text_token_type_quote_level_1_end		= 12,
	text_token_type_left_square_bracket		= 13,
	text_token_type_right_square_bracket	= 14
} text_token_type;

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