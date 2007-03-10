#include <CoreFoundation/CFByteOrder.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <limits.h>
#include <unistd.h>

#include "avi.h"

void die(const char *err);		/* Print error message and exit */

int is_chunk_list(uint32_t id) {
	return id == 'RIFF' || id == 'LIST';
}

avi_chunk read_chunk(void *addr) {
	avi_chunk chunk = *(avi_chunk*)addr;
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

void avi_dump(avi_chunk chunk, int indent, int full) {
	int i, cn;
	
	for (i = 0; i < indent; ++i) printf("  ");
	printf("'%.4s' - ", (char*)&chunk.chunk_id);
	
	if (is_chunk_list(chunk.chunk_id)) {
		printf("'%.4s'\n", (char*)&chunk.list_id);
		avi_chunk child = first_child;
		cn = 0;
		while (next_chunk_child(&chunk, &child)) {
			if (!full && cn) {
				if (is_frame_id(&child.chunk_id) || (child.chunk_id == 'LIST'
						&& child.list_id == 'rec ')) {
					for (i = 0; i < indent + 1; ++i) printf("  ");
					printf(" ...\n");
					break;
				}
			}
			
			avi_dump(child, indent + 1, full);
			if (child.chunk_id == 0xFFFFFFFF) break;
			++cn;
		}
	} else {
		printf("%u bytes\n", chunk.size);
	}
}

avi_file avi_file_read_name(const char *name) {
	int fd;
	
	fd = open(name, O_RDONLY);
	if (fd == -1) die("Can't open avi file");
	return avi_file_read(fd);
}

avi_file avi_file_read(int fd) {
	struct stat sb;
	int err;
	avi_file file;
	char *multi;
	
	err = fstat(fd, &sb);
	if (err == -1) die("Can't get size of input file");
	file.size = sb.st_size;
	if (file.size < 16) die("Input file too small");
	
	file.map = mmap(NULL, file.size, PROT_READ, MAP_FILE, fd, 0);
	if (file.map == (void*)-1) die("Can't map input file");
	
	file.root = read_chunk(file.map);
	if (file.root.chunk_id != 'RIFF' || file.root.list_id != 'AVI ')
		die("Input isn't an avi file!");
	
	/* Some avi files are really groups of RIFFs, ugh */
	multi = (char*)file.root.data + file.root.size;
	if (multi < (char*)file.map + file.size - 4
			&& *(uint32_t*)multi == 'RIFF') {
		file.root.data = file.map;
		file.root.size = file.size;
	}
	
	return file;
}

avi_header avi_get_header(avi_file file) {
	avi_chunk root;
	avi_chunk child;
	avi_header hdr = { };
	
	root = file.root;
	
	/* Deal with multis */
	if (root.data == file.map) {
		child = first_child;
		if (!next_chunk_child(&root, &child))
			die("Can't find real root from multi");
		root = child;
	}
	
	hdr.file = file;
	child = first_child;
	while (next_chunk_child(&root, &child)) {
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

avi_writer avi_write_start(avi_header *hdr, int writefd) {
	avi_writer wr;
	uint32_t hdr_size;
	
	hdr_size = (char*)hdr->movi.data - (char*)hdr->file.map;
	
	wr.writefd = writefd;
	wr.frames_start = hdr_size;
	wr.frame_data_written = 0;
	
	if (write(writefd, hdr->file.map, hdr_size) != hdr_size)
		die("Can't write header");
	return wr;
}

void avi_write_frames(avi_writer *wr, void *start, void *end) {
	uint32_t size;
	
	size = (char*)end - (char*)start;
	if (!size) return;
	
	if (write(wr->writefd, start, size) != size)
		die("Error writing frames");
	
	wr->frame_data_written += size;
}

void avi_write_finish(avi_writer *wr) {
	uint32_t tot_size_le, frames_size_le;
	int fd;
	
	fd = wr->writefd;
	tot_size_le = CFSwapInt32HostToLittle(wr->frame_data_written +
		wr->frames_start);
	frames_size_le = CFSwapInt32HostToLittle(wr->frame_data_written);
	
	if (lseek(fd, 4, SEEK_SET) == -1) die("Can't seek to file size");
	if (write(fd, &tot_size_le, 4) != 4) die("Can't write file size");
	
	if (lseek(fd, wr->frames_start - 8, SEEK_SET) == -1)
		die("Can't seek to frames size");
	if (write(fd, &frames_size_le, 4) != 4) die("Can't write frames size");
	
	close(fd);
}

void avi_find_frames(avi_header *hdr, uint32_t offset, uint32_t maxsize,
		avi_writer *wr) {
	uint32_t *p;
	avi_chunk chunk;
	char *addr, *fileend, *end, *start, *next;
	int skip;
	uint32_t blocksz;
	
	p = (uint32_t*)((char*)hdr->file.map + offset);
	/* Can't be earlier than the movi data */
	if (p < (uint32_t*)hdr->movi.data) p = hdr->movi.data;
	
	fileend = (char*)hdr->file.map + hdr->file.size;
	end = (char*)p + maxsize;
	if (!maxsize || fileend < end) end = fileend;
	
	/* Find an initial frame */
	addr = 0;
	for (; p < (uint32_t*)end; ++p) {
		if (is_frame_id(p)) {
			addr = (char*)p;
			break;
		}
	}
	if (!addr) die("Can't find any frames");
	
	
	/* Follow the frames */
	blocksz = 1024 * 1024 * 64;
	start = addr;
	while (addr < end) {
		chunk = read_chunk(addr);
		if (chunk.chunk_id == 'LIST') {
			skip = (chunk.list_id != 'rec ');
			next = chunk.data;
		} else if (chunk.chunk_id == 0xFFFFFFFF) { /* damage */
			break;
		} else {
			skip = !is_frame_id(&chunk.chunk_id);
			next = (char*)chunk.data + chunk.size;
			if ((uint32_t)next & 0x1) ++next; /* align to 2-bytes */
		}
		
		if (start) {
			if (skip) { /* write until skip point */
				avi_write_frames(wr, start, addr);
				start = 0;
			} else if (addr > start + blocksz) { /* time to write a block */
				avi_write_frames(wr, start, next);
				start = next;
			}
		} else if (!skip) { /* make a note that we're ok to write this */
			start = addr;
		}
		
		addr = next;
	}
	if (addr > fileend) addr = fileend;
	if (start) avi_write_frames(wr, start, addr);
}

void avipart(avi_file file, const char *outname, uint32_t offset,
		uint32_t maxsize) {
	avi_header hdr;
	avi_writer wr;
	int outfd;
	
	outfd = open(outname, O_RDWR | O_CREAT | O_TRUNC, 0666);
	if (outfd == -1) die("Can't open output file");
	
	hdr = avi_get_header(file);
	wr = avi_write_start(&hdr, outfd);
	avi_find_frames(&hdr, offset, maxsize, &wr);
	avi_write_finish(&wr);
	
	munmap(file.map, file.size);
}

void die(const char *err) {
	int saveerr = errno;
	fprintf(stderr, "Error: %s\n", err);
	if (saveerr) fprintf(stderr, "%s\n", strerror(saveerr));
	exit(-2);
}
