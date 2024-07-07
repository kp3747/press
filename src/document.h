typedef enum
{
	document_element_type_heading_1,
	document_element_type_heading_2,
	document_element_type_heading_3,
	document_element_type_text_block,
	document_element_type_line_break,
	document_element_type_preformatted,
	document_element_type_paragraph_end,
	document_element_type_unordered_list,
	document_element_type_blockquote_end,
	document_element_type_paragraph_begin,
	document_element_type_blockquote_begin,
	document_element_type_ordered_list_end,
	document_element_type_ordered_list_item,
	document_element_type_paragraph_break_begin,
	document_element_type_ordered_list_begin_roman,
	document_element_type_ordered_list_begin_arabic,
	document_element_type_ordered_list_begin_letter
} document_element_type;

typedef struct
{
	uint32_t day	: 5;
	uint32_t month	: 4;
	uint32_t year	: 23;
} date;

typedef struct
{
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
	const char*	text;
} document_reference;

typedef struct
{
	document_element*	elements;
	document_reference*	references;
	uint32_t			element_count;
	uint32_t			reference_count;
} document_chapter;

typedef struct
{
	document_metadata	metadata;
	document_chapter*	chapters;
	uint32_t			chapter_count;
} document;