#include "format_internal.h"

#include <cerrno>

namespace fmt {

template <typename Exc> void throwf(const char *fmt, ...) {
    FMT_STRING(s)
    throw(Exc(s));
}

template <typename Exc> void perrorf(const char *fmt, ...) {
    int saveerr = errno;
    FMT_STRING(s)
    
    static size_t bufsize = 256;
    static char* buf = new char[bufsize];
    
    if (saveerr) {
        do {
            int err = strerror_r(saveerr, buf, bufsize);
            if (err == ERANGE) {
                bufsize *= 2;
                delete[] buf;
                buf = new char[bufsize];
            } else {
                break;
            }
        } while (true);
        s = stringf("%s: %s", s.c_str(), buf);
    }
    throw(Exc(s));
}

}
