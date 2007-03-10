#include "avi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *progname;

void usage(const char *err) {
	if (err) fprintf(stderr, "%s\n\n", err);
	fprintf(stderr, "Usage: %s [-full] input.avi\n", progname);
	fprintf(stderr, "Dump the structure of an avi file.\n");
	exit(-1);
}	

int main(int argc, char *argv[]) {
	avi_file avi;
	int full;
	char *input;
	
	progname = argv[0];
	
	full = 0;
	input = NULL;
	if (argc == 2) {
		input = argv[1];
	} else if (argc == 3) {
		input = argv[2];
		if (strcmp(argv[1], "-full") == 0) {
			full = 1;
		} else {
			usage(NULL);
		}
	} else {
		usage(NULL);
	}
	
	avi = avi_file_read_name(input);
	avi_dump(avi.root, 0, full);
	
	return 0;
}
