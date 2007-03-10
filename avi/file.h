#ifndef __AVI_FILE_H__
#define __AVI_FILE_H__

#include "avi/id.h"
#include "avi/mmap.h"
#include "avi/chunk_base.h"

#include <stdint.h>
#include <string>

namespace avi {

class file {
    mmap m_map;
    
public:
    static const id ID;
    static const id LIST_ID;
    
    file(const std::string& name);
    virtual ~file() { }
};

}

#endif
