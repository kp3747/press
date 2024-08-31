void	mem_init(void);
void	mem_term(void);
void*	mem_push(void);
void	mem_pop(void* frame);
void*	mem_alloc(int64_t size);