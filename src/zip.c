// https://pkware.cachefly.net/webdocs/APPNOTE/APPNOTE-6.3.9.TXT

static uint32_t crc32_compute_buffer(uint32_t crc_in, const void* buffer, size_t size);

#pragma pack(push, 1)
typedef struct
{
	char		signature[4];					// end of central dir signature (0x06054B50)										50 4B 05 06
	uint16_t	disk_index;						// number of this disk																00 00
	uint16_t	central_directory_disk_index;	// number of the disk with the start of the central directory						00 00
	uint16_t	disk_entry_count;				// total number of entries in the central directory on this disk					04 00		(4)
	uint16_t	total_entry_count;				// total number of entried in the central directory									04 00		(4)
	uint32_t	central_directory_size;			// size of the central directory													EA 00 00 00	(234)
	uint32_t	offset;							// offset of start of central directory with respect to the starting disk number	17 2F 00 00	(12055)
	uint16_t	comment_len;					// .ZIP file comment length															00 00
	//char		comment[];						// .ZIP file comment (variable size)
} zip_end_of_central_directory_record;
#pragma pack(pop)
static_assert(sizeof(zip_end_of_central_directory_record) == 22);

#pragma pack(push, 1)
typedef struct
{
	char		signature[4];				// central file header signature (0x02014B50)	50 4B 01 02
	uint16_t	version_made_by;			// version made by								14 00		(Version 2.0 made on DOS)
	uint16_t	version_needed_to_extract;	// version needed to extract					14 00		(Version 2.0)
	uint16_t	flags;						// general purpose bit flag						00 00
	uint16_t	compression_type;			// compression method							00 00
	uint16_t	last_file_time;				// last mod file time							80 OE
	uint16_t	last_file_date;				// last mod file date							F5 58
	uint32_t	crc32;						// crc-32										5E C6 32 0C
	uint32_t	compressed_size;			// compressed size								27 00 00 00 (39)
	uint32_t	uncompressed_size;			// uncompressed size							27 00 00 00 (39)
	uint16_t	filename_len;				// file name length								08 00		(8)
	uint16_t	extra_field_len;			// extra field length							00 00
	uint16_t	comment_len;				// file comment length							00 00
	uint16_t	disk_number_start;			// disk number start							00 00
	uint16_t	internal_file_attributes;	// internal_file_attributes						01 00		(text file)
	uint32_t	external_file_attributes;	// external file attributes						20 00 00 00
	uint32_t	local_header_offset;		// relative offset of local header				00 00 00 00
	//char		filename[];					// file name (variable size)					6D 69 6D 65 74 79 70 65 (mimetype)
	//char		extra_field[];				// extra field (variable size)
	//char		comment[];					// file comment (variable size)
} zip_central_directory_header;
#pragma pack(pop)
static_assert(sizeof(zip_central_directory_header) == 46);

#pragma pack(push, 1)
typedef struct
{
	char		signature[4];		// local file header signature (0x04034B50)	50 4B 03 04
	uint16_t	version;			// version needed to extract				14 00
	uint16_t	flags;				// general purpose bit flag					00 00
	uint16_t	compression_type;	// commpression method						00 00
	uint16_t	last_file_time;		// last mod file time						80 0E
	uint16_t	last_file_date;		// last mod file date						F5 58
	uint32_t	crc32;				// crc-32									5E C6 32 0C
	uint32_t	compressed_size;	// compressed size							27 00 00 00
	uint32_t	uncompressed_size;	// uncompressed size						27 00 00 00
	uint16_t	filename_len;		// file name length							08 00
	uint16_t	extra_field_len;	// extra field length						00 00
	//char		filename[];			// file name (variable size)				6D 69 6D 65 74 79 70 65 (mimetype)
	//char		extra_field[];		// extra field (variable size)
} zip_local_file_header;
#pragma pack(pop)
static_assert(sizeof(zip_local_file_header) == 30);

void get_dos_date_time(uint16_t* out_date, uint16_t* out_time)
{
	time_t t = time(nullptr);
	struct tm* gm = gmtime(&t);

	int year = gm->tm_year + 80;	// DOS times start from 1980 instead of 1900

	uint16_t dos_date = 0;
	dos_date |= year << 9;			// Bits 9-15
	dos_date |= gm->tm_mon << 4;	// Bits 5-8
	dos_date |= gm->tm_mday;		// Bits 0-4

	uint16_t dos_time = 0;
	dos_time |= gm->tm_hour << 11;	// Bits 11-15
	dos_time |= gm->tm_min << 5;	// Bits 5-10
	dos_time |= gm->tm_sec / 2;		// Bits 0-4

	*out_date = dos_date;
	*out_time = dos_time;
}

static void generate_zip(const char* filepath, const char** input_files, const char** output_files, uint32_t count)
{
	// Cache string lengths
	uint16_t* filename_lengths = malloc(sizeof(uint16_t) * count);
	for (uint32_t i = 0; i < count; ++i)
		filename_lengths[i] = (uint16_t)strlen(output_files[i]);

	// Open files and cache sizes
	FILE** file_handles = malloc(sizeof(FILE*) * count);
	uint32_t* file_sizes = malloc(sizeof(uint32_t) * count);
	for (uint32_t i = 0; i < count; ++i)
	{
		file_handles[i] = open_file(input_files[i], file_mode_read);
		file_sizes[i] = get_file_size(file_handles[i]);
	}

	// Calculate zip file size
	uint64_t zip_size = 0;

	// Single end of central directory record
	zip_size += sizeof(zip_end_of_central_directory_record);

	// The rest of the size depends on the file count, names, and content
	for (uint32_t i = 0; i < count; ++i)
	{
		zip_size += sizeof(zip_local_file_header);
		zip_size += sizeof(zip_central_directory_header);
		zip_size += (filename_lengths[i] * 2);
		zip_size += file_sizes[i];
	}

	// Make sure it isn't too big
	if (zip_size >= UINT32_MAX)
		handle_error("Output \"%s\" too large.", filepath);

	// Allocate space for entire zip file
	uint8_t* zip_data = malloc(zip_size);

	// Get time and date
	uint16_t date;
	uint16_t time;
	get_dos_date_time(&date, &time);

	uint16_t central_directory_size = 0;
	for (uint32_t i = 0; i < count; ++i)
	{
		central_directory_size += sizeof(zip_central_directory_header);
		central_directory_size += filename_lengths[i];
	}

	uint8_t* current_data = zip_data;
	zip_end_of_central_directory_record* ecdr = (zip_end_of_central_directory_record*)(zip_data + zip_size - sizeof(zip_end_of_central_directory_record));
	zip_central_directory_header* current_dir = (zip_central_directory_header*)((uint8_t*)ecdr - central_directory_size);

	ecdr->signature[0]					= 0x50;
	ecdr->signature[1]					= 0x4B;
	ecdr->signature[2]					= 0x05;
	ecdr->signature[3]					= 0x06;
	ecdr->disk_index					= 0x0000;
	ecdr->central_directory_disk_index	= 0x0000;
	ecdr->disk_entry_count				= count;
	ecdr->total_entry_count				= count;
	ecdr->central_directory_size		= central_directory_size;
	ecdr->offset						= (uint32_t)((uint8_t*)current_dir - zip_data);

	for (uint32_t i = 0; i < count; ++i)
	{
		zip_local_file_header* local = (zip_local_file_header*)current_data;
		current_data += sizeof(zip_local_file_header);

		// File path directory follows header
		memcpy(current_data, output_files[i], filename_lengths[i]);
		current_data += filename_lengths[i];

		// Read file and calculate CRC32 from memory
		fread(current_data, 1, file_sizes[i], file_handles[i]);
		const uint32_t crc = crc32_compute_buffer(0, current_data, file_sizes[i]);
		current_data += file_sizes[i];

		local->signature[0]			= 0x50;
		local->signature[1]			= 0x4B;
		local->signature[2]			= 0x03;
		local->signature[3]			= 0x04;
		local->version				= 0x0014;	// Version 2.0
		local->compression_type		= 0x0000;	// No Compression
		local->last_file_time		= time;
		local->last_file_date		= date;
		local->crc32				= crc;
		local->compressed_size		= file_sizes[i];
		local->uncompressed_size	= file_sizes[i];
		local->filename_len			= filename_lengths[i];
		local->extra_field_len		= 0x0000;

		current_dir->signature[0]				= 0x50;
		current_dir->signature[1]				= 0x4B;
		current_dir->signature[2]				= 0x01;
		current_dir->signature[3]				= 0x02;
		current_dir->version_made_by			= 0x0014;
		current_dir->version_needed_to_extract	= 0x0014;
		current_dir->flags						= 0x0000;
		current_dir->compression_type			= 0x0000;
		current_dir->last_file_time				= time;
		current_dir->last_file_date				= date;
		current_dir->crc32						= crc;
		current_dir->compressed_size			= file_sizes[i];
		current_dir->uncompressed_size			= file_sizes[i];
		current_dir->filename_len				= filename_lengths[i];
		current_dir->extra_field_len			= 0x0000;
		current_dir->comment_len				= 0x0000;
		current_dir->disk_number_start			= 0x0000;
		current_dir->internal_file_attributes	= 0x0001;
		current_dir->external_file_attributes	= 0x00000020;
		current_dir->local_header_offset		= (uint32_t)((uint8_t*)local - zip_data);
		memcpy(current_dir + 1, output_files[i], filename_lengths[i]);

		current_dir = (zip_central_directory_header*)((uint8_t*)current_dir + sizeof(zip_central_directory_header) + filename_lengths[i]);
	}

	// Write out entire file
	FILE* zip_file = open_file(filepath, file_mode_write);
	fwrite(zip_data, zip_size, 1, zip_file);
	fclose(zip_file);
}