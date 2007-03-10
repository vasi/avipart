#include "avi.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>


static char *progname; /* The name of this program */

void usage(const char *err);	/* Print usage and exit */

/* Turn a size string  (eg: 100M) into a number of bytes */
uint32_t size_to_bytes(const char *psize);

/* Parse the program arguments */
void parse_args(int argc, char **argv,
		char **outinfile, char **outoutfile, uint32_t *outoffset);


void usage(const char *err) {
	if (err) fprintf(stderr, "%s\n\n", err);
	fprintf(stderr, "Usage: %s input.avi output.avi PIECE-SIZE START-PIECE\n",
		progname);
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

int main(int argc, char *argv[]) {
	char *input, *output;
	uint32_t offset;
	avi_file inavi;
	
	parse_args(argc, argv, &input, &output, &offset);
	
	inavi = avi_file_read_name(input);
	avipart(inavi, output, offset, 0);
	
	return 0;
}
