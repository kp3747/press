enum
{
	page_size = 4 << 10,
	vmem_size = 64 << 20
};
static_assert((page_size & (page_size - 1)) == 0); // Ensure power of two
static_assert(vmem_size % page_size == 0); // Ensure multiple of page size

static uint8_t* mem_begin;
static uint8_t* mem_current;
static uint8_t* mem_mapped;

static uint8_t* mem_align_page(uint8_t* ptr)
{
	return (uint8_t*)(((uint64_t)ptr + (page_size - 1)) & ~(page_size - 1));
}

static int64_t mem_align_size(int64_t size)
{
	// 8 byte alignment handles everything in this app
	return (size + 7) & ~7;
}

static void mem_init(void)
{
	mem_begin = mem_current = mem_mapped = VirtualAlloc(nullptr, vmem_size, MEM_RESERVE, PAGE_READWRITE);
	if (!mem_current)
		print_error("Unable to reserve %d MiB virtual memory.", vmem_size);
}

static void mem_term(void)
{
	assert(mem_current == mem_begin);
}

static void* mem_push(void)
{
	return mem_current;
}

static void mem_pop(void* frame)
{
	assert((uint8_t*)frame >= mem_begin);
	assert((uint8_t*)frame <= mem_current);

	mem_current = frame;
}

static void* mem_alloc(int64_t size)
{
	size = mem_align_size(size);

	uint8_t* ptr = mem_current;
	uint8_t* next = mem_current + size;

	if (next > mem_mapped)
	{
		uint8_t* current_mapped = mem_mapped;
		mem_mapped = mem_align_page(next);

		if (mem_mapped - mem_begin > vmem_size)
			print_error("Reserved memory of size %d MiB exceeded. Please contact developer.", vmem_size);

		const int64_t map_size = mem_mapped - current_mapped;
		current_mapped = VirtualAlloc(current_mapped, map_size, MEM_COMMIT, PAGE_READWRITE);
		if (!current_mapped)
			print_error("Out of memory.", vmem_size);
	}

	mem_current = next;

	return ptr;
}