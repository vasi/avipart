#ifndef __AVI_ID_H__
#define __AVI_ID_H__

#include <stdint.h>
#include <string>

namespace avi {

class id {
    typedef uint32_t type;
    static const type DAMAGED_ID_VALUE;
    
    type m_val;
    
public:
    id(type i = DAMAGED_ID_VALUE);
    
    type value() const;
    std::string string() const;
    const char *chars() const; // doesn't include trailing \0 !
    
    bool operator==(const id& other) const;
    bool operator!=(const id& other) const;
};

}

#endif
