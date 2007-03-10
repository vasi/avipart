#ifndef __AVI_FILE_H__
#define __AVI_FILE_H__

#include "avi/id.h"
#include "avi/list.h"
#include "avi/mmap.h"
#include "avi/chunk_base.h"

#include <stdint.h>
#include <string>

namespace avi {

class file : public list {
    mmap m_map;
    
public:
    static const id ID;
    static const id LIST_ID;
    
    file(const std::string& name);
    virtual ~file() { }

protected:
    virtual const mmap& map() const;
    virtual offset list_data_offset() const;
    virtual size_t list_data_size() const;
};

}

#endif
