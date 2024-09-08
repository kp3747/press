/*
	These values must remain below 32 (space), and suported ASCII characters must line up with ASCII
	value. Currently these are null, tab, and newline.
*/
typedef enum
{
	text_token_type_null					= 0,
	text_token_type_en_dash					= 1,
	text_token_type_em_dash					= 2,
	text_token_type_note					= 3,
	text_token_type_strong_end				= 4,
	text_token_type_emphasis_end			= 5,
	text_token_type_preformatted			= 6,
	text_token_type_strong_begin			= 7,
	text_token_type_emphasis_begin			= 8,
	text_token_type_tab						= 9,
	text_token_type_newline					= 10,
	text_token_type_quote_level_1_begin		= 11,
	text_token_type_quote_level_1_end		= 12,
	text_token_type_quote_level_2_begin		= 13,
	text_token_type_quote_level_2_end		= 14,
	text_token_type_left_square_bracket		= 15,
	text_token_type_right_square_bracket	= 16,
	text_token_type_fraction_slash			= 17
} text_token_type;

typedef enum
{
	document_element_type_dinkus,
	document_element_type_heading_1,
	document_element_type_heading_2,
	document_element_type_heading_3,
	document_element_type_list_item,
	document_element_type_text_block,
	document_element_type_line_break,
	document_element_type_preformatted,
	document_element_type_paragraph_end,
	document_element_type_blockquote_end,
	document_element_type_paragraph_begin,
	document_element_type_blockquote_begin,
	document_element_type_ordered_list_end,
	document_element_type_unordered_list_end,
	document_element_type_blockquote_citation,
	document_element_type_right_aligned_begin,
	document_element_type_unordered_list_begin,
	document_element_type_paragraph_break_begin,
	document_element_type_ordered_list_begin_roman,
	document_element_type_ordered_list_begin_arabic,
	document_element_type_ordered_list_begin_letter
} document_element_type;

typedef enum
{
	document_type_none,
	document_type_book,
	document_type_article
} document_type;

typedef struct
{
	uint32_t day	: 5;
	uint32_t month	: 4;
	uint32_t year	: 23;
} date;

typedef struct
{
	document_type	type;
	const char*		title;
	const char**	authors;
	const char**	translators;
	date			written;
	date			published;
	uint32_t		author_count;
	uint32_t		translator_count;
} document_metadata;

typedef struct
{
	document_element_type	type;
	const char*				text;
} document_element;

typedef struct
{
	document_element*	elements;
	uint32_t			element_count;
} document_note;

typedef struct
{
	document_element*	elements;
	document_note*		notes;
	uint32_t			element_count;
	uint32_t			note_count;
} document_chapter;

typedef struct
{
	document_metadata	metadata;
	document_chapter*	chapters;
	uint32_t			chapter_count;
} document;