#include <windows.h>

static char file_path[MAX_PATH];

static void kpress_on_error(const char* str)
{
	MessageBoxA(nullptr, str, nullptr, MB_OK | MB_ICONERROR);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, char* cmd_line, int nCmdShow)
{
	install_error_handler(kpress_on_error);

	// Open file dialog
	OPENFILENAME ofn = {
		.lStructSize	= sizeof(OPENFILENAME),
		.hInstance		= instance,
		.lpstrFilter	= "Press-formatted text files (*.txt)\0*.txt\0\0",
		.lpstrFile		= file_path,
		.nMaxFile		= sizeof(file_path),
		.Flags			= OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR
	};
	if (!GetOpenFileNameA(&ofn))
		return 0;

	char* text = load_file(file_path);

	document doc = {};

	line_tokens tokens;
	tokenise(text, &tokens, &doc.metadata);

	// Default to article to allow small documents without any metadata
	if (doc.metadata.type == document_type_none)
		doc.metadata.type = document_type_article;

	doc_mem_req mem_req;
	validate(&tokens, &mem_req);

	finalise(&tokens, &mem_req, &doc);

	if (!doc.metadata.title)
		doc.metadata.title = copy_filename(file_path);

	generate_odt(&doc);
	generate_html(&doc);
	generate_epub(&doc);

	// Open output directory in file explorer
	system("explorer press_output");

	return 0;
}