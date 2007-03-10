#include "format.h"

#include "format_internal.h"

namespace fmt {

std::string vstringf(const char *fmt, std::va_list ap) {
    char *cstr;
    int err = vasprintf(&cstr, fmt, ap); // not in std!?
    if (err == -1) throw std::bad_alloc();
    
    std::string str(cstr);
    std::free(cstr);
    return str;
}


std::string stringf(const char *fmt, ...) {
    FMT_STRING(s)
    return s;
}

}
