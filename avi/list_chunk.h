#ifndef __AVI_LIST_CHUNK_H__
#define __AVI_LIST_CHUNK_H__

#include "avi/chunk_base.h"
#include "avi/list.h"

namespace avi {

class list_chunk : public chunk_base, public list {
    id m_list_id;
    
public:
    static const id ID;
    
    list_chunk(const mmap& map, offset off);
    
    id list_id() const;

protected:
    virtual const mmap& map() const;
    virtual offset list_data_offset() const;
    virtual size_t list_data_size() const;
};

}

#endif
