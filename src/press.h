#include <time.h>
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

enum
{
	page_size = 2 << 20
};

static_assert((page_size & (page_size - 1)) == 0); // Ensure power of two

#define OUTPUT_DIR "press_output"
static const char output_dir[] = OUTPUT_DIR;
enum
{
	output_len = sizeof(output_dir) - 1
};