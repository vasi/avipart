#ifndef __AVI_CHUNK_BASE_H__
#define __AVI_CHUNK_BASE_H__

#include "avi/id.h"
#include "avi/mmap.h"

namespace avi {

typedef uint32_t size; // litte endian

struct mem_chunk_header {
    id chunk_id;
    size decl_size;
};

class chunk_base { // abstract
protected:
    const mmap& m_map;
    
    id m_chunk_id;
    size_t m_total_size;
    size_t m_data_size;
    offset m_data_offset;
    
    // Call in subclass constructor. Data_start should be the offset from
    // 'off' to the start of the chunk's data.
    void init(offset off, const mem_chunk_header *mem, size_t data_start);
    
public:
    chunk_base(const mmap& map);
    virtual ~chunk_base() = 0;
    
    id chunk_id() const;
    bool is_list() const;
        
    size_t total_size() const;
    size_t data_size() const;
    const void *data() const;
};

}

#endif
