#include <CoreFoundation/CFByteOrder.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>


/***** Data structures *****/

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


/***** Declarations *****/

static char *progname; /* The name of this program */

void usage(const char *err);	/* Print usage and exit */
void die(const char *err);		/* Print error message and exit */

/* Turn a size string  (eg: 100M) into a number of bytes */
uint32_t size_to_bytes(const char *psize);

/* Parse the program arguments */
void parse_args(int argc, char **argv,
		char **outinfile, char **outoutfile, uint32_t *outoffset);


int is_chunk_list(uint32_t id);
avi_chunk read_chunk(void *addr);


/***** Argument parsing *****/

void usage(const char *err) {
	if (err) fprintf(stderr, "%s\n\n", err);
	fprintf(stderr, "Usage: %s input.avi output.avi PIECE-SIZE START-PIECE\n",
		progname);
	fprintf(stderr, "Extract a part of an avi file.\n");
	exit(-1);
}	

void die(const char *err) {
	int saveerr = errno;
	fprintf(stderr, "Error: %s\n", err);
	if (saveerr) fprintf(stderr, "%s\n", strerror(saveerr));
	exit(-2);
}

uint32_t size_to_bytes(const char *psize) {
	char *endp;
	uint32_t b, mult;
	
	errno = 0;
	b = strtoul(psize, &endp, 10);
	if (*psize == '\0' || errno) usage("Piece size is not a valid size");
	if (endp[0] == '\0') return b;
	
	mult = 0;
	switch (tolower(endp[0])) {
		case 'k': mult = 1024; break;
		case 'm': mult = 1024 * 1024; break;
		case 'g': mult = 1024 * 1024 * 1024; break;
	}
	if (endp[1] != '\0' || mult == 0)
		usage("Unknown size suffix, use one of K, M, G");
	return mult * b;
}

void parse_args(int argc, char **argv, char **outinfile, char **outoutfile,
		uint32_t *outoffset) {
	char *psizestr, *pnumstr, *endp;
	uint32_t psize, pnum;
	
	progname = argv[0];
	if (argc == 1) usage(NULL);
	if (argc != 5) usage("Four arguments required");
	*outinfile = argv[1], *outoutfile = argv[2];
	
	psizestr = argv[3], pnumstr = argv[4];
	psize = size_to_bytes(psizestr);
	errno = 0;
	pnum = strtoul(pnumstr, &endp, 10);
	if (*pnumstr == '\0' || *endp != '\0' || errno)
		usage("Piece number is not a valid number");
	*outoffset = pnum * psize;
}


int is_chunk_list(uint32_t id) {
	return id == 'RIFF' || id == 'LIST';
}

avi_chunk read_chunk(void *addr) {
	avi_chunk chunk = *(avi_chunk*)addr;
	chunk.addr = addr;
	chunk.size = CFSwapInt32LittleToHost(chunk.size);
	if (is_chunk_list(chunk.chunk_id)) {
		chunk.data = (char*)addr + 12;
		chunk.size -= 4;
	} else {
		chunk.data = (char*)addr + 8;
	}
	return chunk;
}

avi_chunk first_child = { 0 };

/* use chunk_id == 0 as a sentinel */
int next_chunk_child(avi_chunk *parent, avi_chunk *child) {
	char *addr;
	uint32_t room, need;
	
	if (child->chunk_id == 0) {
		addr = parent->data;
	} else {
		/* align to 2-byte boundary */
		addr = (char*)child->data + child->size;
		if ((uint32_t)addr & 0x1) ++addr;
	}
	
	room = (char*)parent->data + parent->size - addr;
	if (room == 0) {
		child->chunk_id = 0;
		return 0;
	}
	need = is_chunk_list(*(uint32_t*)addr) ? 12 : 8;
	if (room < need) die("Child chunk overruns parent");
	
	*child = read_chunk(addr);
	return 1;
}

void avi_dump(avi_chunk chunk, int indent) {
	int i;
	
	for (i = 0; i < indent; ++i) printf("  ");
	printf("'%.4s' - ", (char*)&chunk.chunk_id);
	
	if (is_chunk_list(chunk.chunk_id)) {
		printf("'%.4s'\n", (char*)&chunk.list_id);
		avi_chunk child = first_child;
		while (next_chunk_child(&chunk, &child)) {
			avi_dump(child, indent + 1);
			if (child.chunk_id == 0xFFFFFFFF) break;
		}
	} else {
		printf("%u bytes\n", chunk.size);
	}
}

avi_file avi_file_read(int fd) {
	struct stat sb;
	int err;
	avi_file file;
	
	err = fstat(fd, &sb);
	if (err == -1) die("Can't get size of input file");
	file.size = sb.st_size;
	if (file.size < 16) die("Input file too small");
	
	file.map = mmap(NULL, file.size, PROT_READ, MAP_FILE, fd, 0);
	if (file.map == (void*)-1) die("Can't map input file");
	
	file.root = read_chunk(file.map);
	if (file.root.chunk_id != 'RIFF' || file.root.list_id != 'AVI ')
		die("Input isn't an avi file!");
	
	return file;
}

avi_header avi_get_header(avi_file file) {
	avi_chunk child;
	avi_header hdr;
	
	hdr.file = file;
	child = first_child;
	while (next_chunk_child(&file.root, &child)) {
		if (child.chunk_id != 'LIST' || child.list_id != 'movi') continue;
		
		hdr.movi = child;
		return hdr;
	}
	
	die("Can't find the end of the avi header");
	return hdr; /* make compiler happy */
}

int is_frame_id(const uint32_t *p) {
	char *c = (char*)p;
	return ((c[2] == 'w' && c[3] == 'b')
			|| (c[2] == 'd' && (c[3] == 'c' || c[3] == 'b')))
		&& isdigit(c[0]) && isdigit(c[1]);
}

avi_frames avi_find_frames(avi_header *hdr, uint32_t offset) {
	uint32_t *p;
	avi_chunk chunk;
	void *addr, *end;
	avi_frames frames = { 0, 0 };
	
	/* Can't be earlier than the movi data */
	p = (uint32_t*)((char*)hdr->file.map + offset);
	if (p < (uint32_t*)hdr->movi.data) p = hdr->movi.data;
	
	/* Find an initial frame */
	end = (char*)hdr->movi.data + hdr->movi.size;
	for (; p < (uint32_t*)end; ++p) {
		if (is_frame_id(p)) {
			frames.start = p;
			break;
		}
	}
	if (!frames.start) die("Can't find any frames");
	
	/* Follow the frames */
	addr = frames.start;
	while (addr < end) {
		chunk = read_chunk(addr);
		if (chunk.chunk_id == 'LIST' && chunk.list_id == 'rec ') {
			addr = chunk.data;
		} else if (chunk.chunk_id == 0xFFFFFFFF) { /* damage */
			break;
		} else if (is_frame_id(&chunk.chunk_id)) {
			addr = (char*)chunk.data + chunk.size;
			if ((uint32_t)addr & 0x1) ++addr; /* align to 2-bytes */
		} else {
			die("Unexpected chunk id");
		}
	}
	if (addr > end) die("Frames overrun container");
	frames.length = addr - frames.start;
	
	return frames;
}

void avi_write(int fd, avi_header *hdr, avi_frames *frames) {
	uint32_t hdr_size, tot_size_le, frames_size_le;
	
	hdr_size = (char*)hdr->movi.data - (char*)hdr->file.map;
	tot_size_le = CFSwapInt32HostToLittle(hdr_size + frames->length - 8);
	frames_size_le = CFSwapInt32HostToLittle(frames->length);
	
	if (write(fd, hdr->file.map, hdr_size) != hdr_size)
		die("Can't write header");
	
	if (lseek(fd, 4, SEEK_SET) == -1) die("Can't seek to file size");
	if (write(fd, &tot_size_le, 4) != 4) die("Can't write file size");
	
	if (lseek(fd, -8, SEEK_END) == -1) die("Can't seek to frames size");
	if (write(fd, &frames_size_le, 4) != 4) die("Can't write frames size");
	
	if (lseek(fd, 0, SEEK_END) == -1) die("Can't seek to end");
	if (write(fd, frames->start, frames->length) != frames->length)
		die("Can't write frames");
	
	close(fd);
}

void avipart(int infd, int outfd, uint32_t offset) {
	avi_file file;
	avi_header hdr;
	avi_frames frames;
	
	file = avi_file_read(infd);	
	hdr = avi_get_header(file);
	frames = avi_find_frames(&hdr, offset);
	avi_write(outfd, &hdr, &frames);
	
	munmap(file.map, file.size);
	close(infd);
}

int main(int argc, char *argv[]) {
	char *input, *output;
	uint32_t offset;
	int infd, outfd;
	
	parse_args(argc, argv, &input, &output, &offset);
	
	infd = open(input, O_RDONLY);
	if (infd == -1) die("Can't open input file");
	outfd = open(output, O_RDWR | O_CREAT | O_TRUNC, 0666);
	if (outfd == -1) die("Can't open output file");
	
	avipart(infd, outfd, offset);
	
	return 0;
}
