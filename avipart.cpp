#include "avi.h"
#include "endian.h"
#include "format.h"
#include "i18n.h"
#include "util.h"

static char* progname;

// Print usage, return error code
static int usage(const std::string& err = "");

static int usage(const std::string& err) {
	if (!err.empty()) fprintf(stderr, "%s\n\n", err.c_str());
	fprintf(stderr,
        _("Usage: %s input.avi output.avi PIECE-SIZE START-PIECE\n"), progname);
	fprintf(stderr, _("Extract a part of an avi file.\n"));
	return -1;
}

// NOTE: No character encoding support. Yes, I suck.
int main(int argc, char *argv[]) {
	progname = argv[0];
	
	if (argc == 1) return usage();
	if (argc != 5) return usage(_("Four arguments required"));
	std::string infile = argv[1], outfile = argv[2], psizestr = argv[3],
        pnumstr = argv[4];
    
    try {
        avi::offset offset = util::byte_size<avi::offset>(psizestr);
        offset *= util::convert_string<avi::offset>(pnumstr, "integer");
        
        avi::file file(infile);        
    } catch (const std::exception& e) {
        fprintf(stderr, _("Error: %s\n"), e.what());
        return -2;
    }
	
	return 0;
}
