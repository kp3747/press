#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <setjmp.h>
#include <assert.h>
#include <stdbool.h>

#if __STDC_VERSION__ < 202311		// <C23
	#define nullptr ((void*)0)
#endif

#if __STDC_VERSION__ >= 202311		// >=C23
	#define noreturn [[noreturn]]
#elif __STDC_VERSION__ >= 201112	// >=C11
	#define noreturn _Noreturn		// Deprecated in C23
#else
	#define noreturn
#endif

enum
{
	page_size = 2 << 20
};

static_assert((page_size & (page_size - 1)) == 0); // Ensure power of two

typedef enum
{
	document_element_type_heading_2,
	document_element_type_heading_3,
	document_element_type_paragraph,
	document_element_type_blockquote,
	document_element_type_preformatted,
	document_element_type_unordered_list,
	document_element_type_ordered_list_roman,
	document_element_type_ordered_list_arabic
} document_element_type;

typedef enum
{
	ordered_list_type_arabic,
	ordered_list_type_
} ordered_list_type;

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
	const char*	text;
	uint32_t	level;
} document_heading;

typedef struct
{
	const char* text;
} document_paragraph;

typedef struct
{
	const char* text;
} document_preformatted;

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

noreturn
static void handle_parse_error(const char* label, uint32_t line, uint32_t column, const char* format, ...);