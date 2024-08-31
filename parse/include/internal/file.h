typedef enum
{
	file_mode_read,
	file_mode_write
} file_mode;

typedef struct
{
	void* f;
} file;

void		create_dir(const char* dir);
void		delete_dir(const char* dir);
file		open_file(const char* path, file_mode mode);
void		close_file(file f);
uint32_t	get_file_size(file f);
void		read_file(file f, void* dst, uint32_t size);
void		write_file(file f, const void* src, size_t size);
const char*	copy_filename(const char* filepath);
const char*	generate_path(const char* format, ...);