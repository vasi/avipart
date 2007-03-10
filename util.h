#ifndef __UTIL_H__
#define __UTIL_H__

#include <string>

namespace util {

// Convert a string to another type
template <typename T> static T convert_string(const std::string& str,
    const std::string& desc = "", bool whole_string = true,
    std::string* o_remain = 0L);

// Get a total byte size from a human size spec (eg: "100M")
// Currently assume only K, M, G are valid suffices, and refer to powers of 1024
template <typename T> T byte_size(const std::string& str);

}

#include "util.tcc"

#endif
