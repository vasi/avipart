#ifndef __FORMAT_H__
#define __FORMAT_H__

#include <cstdarg>
#include <string>

// Not necessary to include, but likely useful
#include <stdexcept>
#include "i18n.h"

namespace fmt {
    // Create strings with formats
    std::string vstringf(const char *fmt, std::va_list ap);
    std::string stringf(const char *fmt, ...);
    
    // Throw exceptions with formats
    template <typename Exc> void throwf(const char *fmt, ...);
    template <typename Exc> void perrorf(const char *fmt, ...);
}

#include "format.tcc"

#endif
