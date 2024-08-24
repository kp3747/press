typedef enum
{
	metadata_entry_type_type,
	metadata_entry_type_title,
	metadata_entry_type_author,
	metadata_entry_type_authors,
	metadata_entry_type_translator,
	metadata_entry_type_translators,
	metadata_entry_type_written,
	metadata_entry_type_published,
	metadata_entry_count
} metadata_entry_type;

static const char* metadata_strings[] = {
	"Type:",
	"Title:",
	"Author:",
	"Authors:",
	"Translator:",
	"Translators:",
	"Written:",
	"Published:"
};

static void eat_metadata_spaces(tokenise_context* ctx)
{
	peek_state peek;
	peek_init(ctx, &peek);

	for (;;)
	{
		char c = peek_char(ctx, &peek);
		if (c == ' ')
		{
		}
		else if (c == '\t')
		{
		}
		else if (c == '\n')
		{
			handle_tokenise_error(ctx, "New lines are not permitted within metadata tags \"{...}\".");
		}
		else
		{
			return;
		}

		// Advance one character at a time
		get_char(ctx);
	}
}

static void check_metadata_no_value(tokenise_context* ctx, metadata_entry_type entry_type)
{
	const char c = get_char(ctx);
	if (c != '}')
		handle_tokenise_error(ctx, "Metadata attribute \"{%s}\" may not contain extra characters.", metadata_strings[entry_type]);
}

static const char* parse_metadata_text(tokenise_context* ctx)
{
	eat_metadata_spaces(ctx);

	// Save position of string that is about to be written
	const char* text = ctx->write_ptr;

	for (;;)
	{
		char c = get_char(ctx);
		if (c == '\n')
		{
			handle_tokenise_error(ctx, "New lines are not permitted within metadata tags \"{...}\".");
		}
		else if (c == '\t')
		{
			handle_tokenise_error(ctx, "Tabs are not permitted within metadata values.");
		}
		else if (c == '}')
		{
			// Null terminate string
			put_char(ctx, 0);
			break;
		}
		else
		{
			put_char(ctx, c);
		}
	}

	if (*text == 0)
		handle_tokenise_error(ctx, "Metadata text expected.");

	return text;
}

static const char* parse_metadata_list(tokenise_context* ctx, uint32_t* out_count)
{
	eat_metadata_spaces(ctx);

	uint32_t count = 1;
	uint32_t current_len = 0;
	const char* output = ctx->write_ptr;

	for (;;)
	{
		char c = get_char(ctx);
		if (c == ',')
		{
			c = get_char(ctx);
			if (c != ' ')
				handle_tokenise_error(ctx, "Metadata lists must be separated by a single space.");
			else if (!current_len)
				handle_tokenise_error(ctx, "Metadata list items cannot be empty.");

			++count;
			current_len = 0;

			// Add null terminator
			put_char(ctx, 0);
		}
		else if (c == '\n')
		{
			handle_tokenise_error(ctx, "New lines are not permitted within metadata tags \"{...}\".");
		}
		else if (c == '\t')
		{
			handle_tokenise_error(ctx, "Tabs are not permitted within metadata values.");
		}
		else if (c == '}')
		{
			// Add final terminator
			put_char(ctx, 0);

			break;
		}
		else
		{
			++current_len;

			put_char(ctx, c);
		}
	}

	*out_count = count;

	return output;
}

static int parse_metadata_enum(tokenise_context* ctx, const char* name, const char** strings, int count)
{
	eat_metadata_spaces(ctx);

	for (int i = 0; i < count; ++i)
	{
		peek_state peek;
		peek_init(ctx, &peek);

		const char* str = strings[i];
		for (;;)
		{
			const char peeked_char = peek_char(ctx, &peek);

			if (*str == 0)
			{
				if (peeked_char == '}')
				{
					peek_apply(ctx, &peek);
					return i;
				}
				else
				{
					break;
				}
			}
			else if (peeked_char != *str)
			{
				break;
			}

			++str;
		}
	}

	// If we get here then no enum value was matched
	print_error("Unknown metadata value for attribute \"%s\". Valid values are:\n", name);
	for (int i = 0; i < count; ++i)
		print_error("%s\n", strings[i]);
	exit_failure();

	unreachable();
}

static void parse_metadata_type(tokenise_context* ctx)
{
	if (ctx->metadata->type != document_type_none)
		handle_tokenise_error(ctx, "Duplicate \"Type\" metadata attribute.");

	static const char* type_strings[] = {
		"Book",
		"Article"
	};
	const int type_count = sizeof(type_strings) / sizeof(const char*);

	const int value = parse_metadata_enum(ctx, "Type", type_strings, type_count);
	ctx->metadata->type = value + 1;
}

static void parse_metadata_title(tokenise_context* ctx)
{
	if (ctx->metadata->title)
		handle_tokenise_error(ctx, "Duplicate \"Title\" metadata attribute.");

	ctx->metadata->title = parse_metadata_text(ctx);
}

static void parse_metadata_author(tokenise_context* ctx)
{
	if (ctx->authors)
		handle_tokenise_error(ctx, "Only one \"Author\" or \"Authors\" metdadata attribute is permitted.");

	ctx->authors = parse_metadata_list(ctx, &ctx->metadata->author_count);

	if (ctx->metadata->author_count != 1)
		handle_tokenise_error(ctx, "The \"Author\" metadata attribute only allows for a single author; for mulitple authors use \"Authors\" instead.");
}

static void parse_metadata_authors(tokenise_context* ctx)
{
	if (ctx->metadata->authors)
		handle_tokenise_error(ctx, "Only one \"Author\" or \"Authors\" metdadata attribute is permitted.");

	ctx->authors = parse_metadata_list(ctx, &ctx->metadata->author_count);
}

static void parse_metadata_translator(tokenise_context* ctx)
{
	if (ctx->translators)
		handle_tokenise_error(ctx, "Only one \"Translator\" or \"Translators\" metdadata attribute is permitted.");

	ctx->translators = parse_metadata_list(ctx, &ctx->metadata->translator_count);

	if (ctx->metadata->translator_count != 1)
		handle_tokenise_error(ctx, "The \"Translator\" metadata attribute only allows for a single translator; for mulitple translators use \"Translators\" instead.");
}

static void parse_metadata_translators(tokenise_context* ctx)
{
	if (ctx->translators)
		handle_tokenise_error(ctx, "Only one \"Translator\" or \"Translators\" metdadata attribute is permitted.");

	ctx->translators = parse_metadata_list(ctx, &ctx->metadata->translator_count);
}

static void parse_metadata(tokenise_context* ctx, metadata_entry_type entry_type)
{
	switch (entry_type)
	{
	case metadata_entry_type_type:
		parse_metadata_type(ctx);
		break;
	case metadata_entry_type_title:
		parse_metadata_title(ctx);
		break;
	case metadata_entry_type_author:
		parse_metadata_author(ctx);
		break;
	case metadata_entry_type_authors:
		parse_metadata_authors(ctx);
		break;
	case metadata_entry_type_translator:
		parse_metadata_translator(ctx);
		break;
	case metadata_entry_type_translators:
		parse_metadata_translators(ctx);
		break;
//	case metadata_entry_type_written:
//		break;
//	case metadata_entry_type_published:
//		break;
	}
}

static char tokenise_metadata(tokenise_context* ctx, char c)
{
	// Eat initial '{' char
	c = get_char(ctx);

	if (c == ' ')
		handle_tokenise_error(ctx, "Metadata tags \"{...}\" cannot begin with a space.");
	else if (c == '\t')
		handle_tokenise_error(ctx, "Metadata tags \"{...}\" cannot begin with a tab.");

	// Compare text against all metadata strings to determine type
	for (int i = 0; i < metadata_entry_count; ++i)
	{
		const char* name = metadata_strings[i];
		assert(*name);

		// Check first character first
		if (c == *name)
		{
			peek_state peek;
			peek_init(ctx, &peek);

			// Compare subsequent characters
			for (;;)
			{
				// First character has already been checked
				++name;

				if (*name == 0)
				{
					// Metadata index found
					peek_apply(ctx, &peek);
					parse_metadata(ctx, i);

					// Make sure metadata is followed by a new line
					c = get_char(ctx);
					if (c != '\n')
						handle_tokenise_error(ctx, "Metadata tags \"{...}\" must be followed by a new line.");

					return get_char(ctx);
				}

				if (*name != peek_char(ctx, &peek))
					break; // Not equal; try next metadata index
			}
		}
	}

	handle_tokenise_error(ctx, "Unrecognised metadata attribute.");
	return 0;
}

static const char** finalise_list(const char* src, uint32_t count)
{
	if (!count)
		return nullptr;

	// Allocate array
	const char** list = mem_alloc(sizeof(const char**) * count);

	// Assign first list element outside loop to simplify loop
	const char* current = src;
	list[0] = current;

	// Search for null terminators to fill array
	for (uint32_t i = 1; i < count;)
	{
		if (*current++ == 0)
			list[i++] = current;
	}

	return list;
}

static void finalise_metadata(tokenise_context* ctx)
{
	ctx->metadata->authors = finalise_list(ctx->authors, ctx->metadata->author_count);
	ctx->metadata->translators = finalise_list(ctx->translators, ctx->metadata->translator_count);
}