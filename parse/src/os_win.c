#define PAGE_SIZE (INT64_C(4) << 10)
#define VMEM_SIZE (INT64_C(64) << 20)
static_assert((PAGE_SIZE & (PAGE_SIZE - 1)) == 0); // Ensure power of two
static_assert(VMEM_SIZE % PAGE_SIZE == 0); // Ensure multiple of page size

static uint8_t* mem_begin;
static uint8_t* mem_current;
static uint8_t* mem_mapped;

static uint8_t* mem_align_page(uint8_t* ptr)
{
	return (uint8_t*)(((uint64_t)ptr + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1));
}

static int64_t mem_align_size(int64_t size)
{
	// 8 byte alignment handles everything in this app
	return (size + 7) & ~7;
}

void mem_init(void)
{
	mem_begin = mem_current = mem_mapped = VirtualAlloc(nullptr, VMEM_SIZE, MEM_RESERVE, PAGE_READWRITE);
	if (!mem_current)
		print_error("Unable to reserve %d MiB virtual memory.", VMEM_SIZE);
}

void mem_term(void)
{
	assert(mem_current == mem_begin);
}

void* mem_push(void)
{
	return mem_current;
}

void mem_pop(void* frame)
{
	assert((uint8_t*)frame >= mem_begin);
	assert((uint8_t*)frame <= mem_current);

	mem_current = frame;
}

void* mem_alloc(int64_t size)
{
	size = mem_align_size(size);

	uint8_t* ptr = mem_current;
	uint8_t* next = mem_current + size;

	if (next > mem_mapped)
	{
		uint8_t* current_mapped = mem_mapped;
		mem_mapped = mem_align_page(next);

		if (mem_mapped - mem_begin > VMEM_SIZE)
			print_error("Reserved memory of size %d MiB exceeded. Please contact developer.", VMEM_SIZE);

		const int64_t map_size = mem_mapped - current_mapped;
		current_mapped = VirtualAlloc(current_mapped, map_size, MEM_COMMIT, PAGE_READWRITE);
		if (!current_mapped)
			print_error("Out of memory.", VMEM_SIZE);
	}

	mem_current = next;

	return ptr;
}

void create_dir(const char* dir)
{
	const BOOL result = CreateDirectoryA(dir, nullptr);
	if (!result)
		handle_error("Unable to create directory \"%s\". Do you have a previously generated file open?", dir);
}

void delete_dir(const char* dir)
{
	void* frame = mem_push();

	const int64_t len = strlen(dir);
	char* file_path = mem_alloc(len + 2);
	memcpy(file_path, dir, len);

	// The API requires path to end with two null terminators
	file_path[len] = file_path[len + 1] = 0;

	SHFILEOPSTRUCT op = {
		.wFunc	= FO_DELETE,
		.pFrom	= file_path,
		.fFlags	= FOF_NO_UI
	};

	SHFileOperationA(&op);

	mem_pop(frame);
}

#undef PAGE_SIZE
#undef VMEM_SIZE