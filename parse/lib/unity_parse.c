#include "../include/parse.h"

//#include "mem.h"
//#include "document.h"
#include "../src/util.h"
#include "../src/tokenise.h"
#include "../src/validate.h"
#include "../src/finalise.h"
//#include "generate.h"

//#include <time.h>
#include <stdio.h>
//#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
//#include <stdint.h>
//#include <setjmp.h>
//#include <assert.h>
//#include <stdbool.h>

#include "../src/roman_numeral.c"
//#include "odt.c"
//#include "html.c"
//#include "html_css.c"
//#include "epub.c"
#include "../src/validate.c"
#include "../src/finalise.c"
//#include "zip.c"
//#include "crc32.c"
#include "../src/util.c"

#include "../src/tokenise_internal.h"
#include "../src/tokenise_metadata.c"
#include "../src/tokenise.c"

#if defined(_MSC_VER)
	#include "../src/os_win.c"
#else
	#error Unsupported platform
#endif

// TODO: Move platform-specific code into its own file
//#include <windows.h>
//#include "main.c"
//#include "mem.c"