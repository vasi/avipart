#include "format.h"
#include <sstream>

namespace util {

template <typename T> T convert_string(const std::string& str,
        const std::string& desc, bool whole_string, std::string* o_remain) {
    std::istringstream ss(str);
    T t;
    ss >> t;
    
    if (ss.fail() || (whole_string && !ss.eof())) {
        std::string to = desc.empty() ? ""
            : fmt::stringf(_(" to a %s"), desc.c_str());
        std::string conv = ss.fail() ? _("convert") : _("completely convert"); 
        fmt::throwf<std::invalid_argument>(_("Can't %s '%s'%s"), conv.c_str(),
            str.c_str(), to.c_str());
    }
    
    if (o_remain) ss >> *o_remain;
    return t;
}

template <typename T> T byte_size(const std::string& str) {
    std::string suf;
    T bytes = convert_string<T>(str, "integer", false, &suf);
    
    std::transform(suf.begin(), suf.end(), suf.begin(), &tolower);
    if (suf.size() == 2 && suf.at(1) == 'b') // Allow a B in suffix
        suf.resize(1);
    
    static std::string sufs[] = { "", "k", "m", "g" };
    bool found = false;
    for (size_t i = 0; i < sizeof(sufs)/sizeof(sufs[0]); ++i) {
        if (sufs[i] == suf) {
            found = true;
            break;
        }
        bytes *= 1024;
    }
    if (!found)
        fmt::throwf<std::invalid_argument>(
            _("'%s' doesn't seem to be a byte size"), str.c_str());
    return bytes;
}

}
