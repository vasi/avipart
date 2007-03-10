#include <stdint.h>

typedef struct {
	uint32_t chunk_id;
	uint32_t size;
	uint32_t list_id; /* only valid for lists */
	void *data;
	void *addr;
} avi_chunk;

typedef struct {
	void *map;
	uint32_t size;
	avi_chunk root;
} avi_file;

typedef struct {
	avi_file file;
	avi_chunk movi;
} avi_header;

typedef struct {
	void *start;
	uint32_t length;
} avi_frames;

typedef struct {
	int writefd;
	uint32_t frames_start;
	uint32_t frame_data_written;
} avi_writer;


int is_chunk_list(uint32_t id);
int is_frame_id(const uint32_t *p);

avi_chunk read_chunk(void *addr);

extern avi_chunk first_child;
int next_chunk_child(avi_chunk *parent, avi_chunk *child);

void avi_dump(avi_chunk chunk, int indent, int full);

avi_file avi_file_read(int fd);
avi_file avi_file_read_name(const char *name);
avi_chunk avi_real_root(avi_file file);
avi_header avi_get_header(avi_file file);

avi_writer avi_write_start(avi_header *hdr, int writefd);
void avi_write_frames(avi_writer *wr, void *start, void *end);
void avi_write_finish(avi_writer *wr);

void avi_find_frames(avi_header *hdr, uint32_t offset, uint32_t maxsize,
		avi_writer *wr);

void avipart(avi_file file, const char *outname, uint32_t offset,
	uint32_t maxsize);
