#include "avi.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>


static char *progname; /* The name of this program */


void usage(const char *err) {
	if (err) fprintf(stderr, "%s\n\n", err);
	fprintf(stderr, "Usage: %s input.avi output.avi PIECE-SIZE START-PIECE"
		" [MAX-PIECES]\n", progname);
	fprintf(stderr, "Extract a part of an avi file.\n");
	exit(-1);
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

uint32_t strtoul_die(char *str) {
	char *endp, *msg;
	
	errno = 0;
	uint32_t i = strtoul(str, &endp, 10);
	
	if (*str == '\0' || *endp != '\0' || errno) {
		asprintf(&msg, "'%s' is not a number", str);
		usage(msg);
	}
	return i;
}

void parse_args(int argc, char **argv, char **outinfile, char **outoutfile,
		uint32_t *outoffset, uint32_t *maxsize) {
	char *psizestr, *pnumstr;
	uint32_t psize, pnum;
	
	progname = argv[0];
	if (argc < 5) usage("Not enough arguments");
	if (argc > 6) usage("Too many arguments");
	
	*outinfile = argv[1], *outoutfile = argv[2];
	
	psizestr = argv[3], pnumstr = argv[4];
	psize = size_to_bytes(psizestr);
	pnum = strtoul_die(pnumstr);
	
	if (argc == 6)
		*maxsize = strtoul_die(argv[5]) * psize;
	*outoffset = (pnum - 1) * psize;
}

int main(int argc, char *argv[]) {
	char *input, *output;
	uint32_t offset, maxsize;
	avi_file inavi;
	
	maxsize = 0;
	parse_args(argc, argv, &input, &output, &offset, &maxsize);
	
	inavi = avi_file_read_name(input);
	avipart(inavi, output, offset, maxsize);
	
	return 0;
}
