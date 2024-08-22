#include "press.h"
#include "util.h"
#include "document.h"
#include "tokenise.h"
#include "validate.h"
#include "finalise.h"
#include "generate.h"

#include "roman_numeral.c"
#include "odt.c"
#include "html.c"
#include "epub.c"
#include "validate.c"
#include "finalise.c"
#include "zip.c"
#include "crc32.c"

#include "tokenise_internal.h"
#include "tokenise_metadata.c"
#include "tokenise.c"

// TODO: Move platform-specific code into its own file
#include <windows.h>
#include "main.c"
#include "util.c"