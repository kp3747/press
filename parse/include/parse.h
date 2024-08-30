#pragma once

#include <stdint.h>
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

#if NDEBUG
	#define debug_trap() do {} while(0)
#else
	#if defined(_MSC_VER)
		#define debug_trap() do { __debugbreak(); } while(0)
	#elif defined(__clang__)
		#define debug_trap() do { __builtin_trap(); } while(0)
	#else
		#define debug_trap() do { assert(0); } while(0)
	#endif
#endif

#if defined(_MSC_VER)
	#define unreachable() do { __assume(0); debug_trap(); } while(0)
#elif defined(__clang__) && __has_builtin(__builtin_unreachable)
	#define unreachable() do { __builtin_unreachable(); debug_trap(); } while(0)
#else
	#define unreachable() do { debug_trap(); } while(0)
#endif

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
	text_token_type_quote_level_2_begin		= 13,
	text_token_type_quote_level_2_end		= 14,
	text_token_type_left_square_bracket		= 15,
	text_token_type_right_square_bracket	= 16
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

typedef enum
{
	file_mode_read,
	file_mode_write
} file_mode;

typedef struct
{
	void* f;
} file;

document*	parse(const char* filepath);

void		mem_init(void);
void		mem_term(void);
void*		mem_push(void);
void		mem_pop(void* frame);
void*		mem_alloc(int64_t size);

static char*		load_file(const char* filepath);
static void			create_dir(const char* dir);
static void			delete_dir(const char* dir);
static file			open_file(const char* path, file_mode mode);
static void			close_file(file f);
static uint32_t		get_file_size(file f);
static void			read_file(file f, void* dst, uint32_t size);
static void			write_file(file f, const void* src, size_t size);
static const char*	copy_filename(const char* filepath);
static const char*	generate_path(const char* format, ...);

static void			print_str(file f, const char* s);
static void			print_fmt(file f, const char* format, ...);
static void			print_char(file f, char c);
static void			print_char_token(file f, char c);
static void			print_tabs(file f, int depth);
static void			print_en_dash(file f);
static void			print_em_dash(file f);
static void			print_apostrophe(file f);
static void			print_quote_level_1_begin(file f);
static void			print_quote_level_1_end(file f);
static void			print_quote_level_2_begin(file f);
static void			print_quote_level_2_end(file f);
static void			print_simple_text(file f, const char* text);

// TODO: Move these to error.h
typedef void (*error_callback)(const char* str);

static void			install_error_handler(error_callback handler);
static void			handle_error(const char* format, ...);
static void			exit_failure(void);
static void			print_error(const char* format, ...);
static void			print_error_char(char c);
static void			print_error_vargs(const char* format, va_list args);