typedef struct
{
	uint32_t chapter_count;
	uint32_t element_count;
	uint32_t note_count;
	uint32_t note_element_count;
} doc_mem_req;

static void validate(line_tokens* tokens, doc_mem_req* out_mem_req);