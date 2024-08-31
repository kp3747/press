/*
	TODO: Handle any filename length. Perhaps we could just add a global buffer of the maximum
	possible size and reuse between functions? For example 64 KiB * 4 = 256KiB. I believe Windows
	supports ~INT16_MAX for UTF-16. To be safe we could use UINT16_MAX * 4 (max UTF-8 length). For
	reference the max Linux file path is a more reasonable 4096. We could just support that and
	report an error if a path length exceeds that. In fact, that sounds like a good idea.
*/

static char* load_file(const char* filepath)
{
	file f = open_file(filepath, file_mode_read);
	const uint32_t size = get_file_size(f);

	/*
		Allocate enough memory plus two bytes:
		1. Potential extra new line character before null terminator to make parsing simpler.
		2. Null terminator.
	*/
	char* data = mem_alloc(size + 2);

	// Put data one byte past the beginning of the buffer to allow space for initial control code
	fread(data, 1, size, f.f);
	fclose(f.f);

	// Check if final new line character needs to be added, then null terminate
	if (data[size - 1] == '\n')
	{
		data[size] = 0;
	}
	else
	{
		data[size] = '\n';
		data[size + 1] = 0;
	}

	return data;
}

void parse(const char* filepath, document* out_doc)
{
	memset(out_doc, 0x00, sizeof(*out_doc));

	char* text = load_file(filepath);

	line_tokens tokens;
	tokenise(text, &tokens, &out_doc->metadata);

	doc_mem_req mem_req;
	validate(&tokens, &mem_req);

	finalise(&tokens, &mem_req, out_doc);
}