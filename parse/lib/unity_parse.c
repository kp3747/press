#include "../include/parse.h"
#include "../src/tokenise.h"
#include "../src/validate.h"
#include "../src/finalise.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "../src/parse.c"
#include "../src/roman_numeral.c"
#include "../src/validate.c"
#include "../src/finalise.c"
#include "../src/file.c"
#include "../src/print.c"
#include "../src/error.c"

#include "../src/tokenise_internal.h"
#include "../src/tokenise_metadata.c"
#include "../src/tokenise.c"

#if defined(_MSC_VER)
	#include "../src/os_win.c"
#else
	#error Unsupported platform
#endif