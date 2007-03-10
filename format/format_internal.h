#ifndef __FORMAT_INTERNAL_H__
#define __FORMAT_INTERNAL_H__

namespace fmt {

// Make a string from the format arguments, available as 'var'
#define FMT_STRING(var) \
    std::va_list ap; \
    va_start(ap, fmt); \
    std::string var; \
    try { /* Grrr, C++ needs 'finally' */ \
        var = vstringf(fmt, ap); \
        va_end(ap); \
    } catch (const std::exception& e) { \
        va_end(ap); \
        throw; \
    }

}

#endif
