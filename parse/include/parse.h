#pragma once

#include "internal/compiler.h"
#include "internal/mem.h"
#include "internal/file.h"
#include "internal/print.h"
#include "internal/error.h"
#include "internal/document.h"

void parse(const char* filepath, document* out_doc);