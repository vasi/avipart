#ifndef __AVI_LIST_CHUNK_H__
#define __AVI_LIST_CHUNK_H__

#include "avi/chunk_base.h"

namespace avi {

class list_chunk : public chunk_base {
    id m_list_id;
    
public:
    static const id ID;
    
    list_chunk(const mmap& map, offset off);
    
    id list_id() const;
};

}

#endif
