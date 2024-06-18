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

noreturn
static void handle_parse_error(const char* label, uint32_t line, uint32_t column, const char* format, ...);