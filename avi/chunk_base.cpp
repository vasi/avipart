#include "avi/chunk_base.h"

#include "endian.h"

namespace avi {

chunk_base::chunk_base(const mmap& map) : m_map(map) { }
chunk_base::~chunk_base() { }

id chunk_base::chunk_id() const {
    return m_chunk_id;
}

size_t chunk_base::total_size() const {
    return m_total_size;
}

size_t chunk_base::data_size() const {
    return m_data_size;
}

const void *chunk_base::data() const {
    return m_map.data(m_data_offset, m_data_size);
}

bool chunk_base::is_list() const {
    return m_chunk_id == LIST_ID || m_chunk_id == AVI_ID;
}

void chunk_base::init(offset off, const mem_chunk_header *mem,
        size_t data_start) {
    m_chunk_id = mem->chunk_id;
    
    size sz = mem->decl_size;
    sz = endian::fromLE32(sz);
    m_total_size = sz + 8;
    
    m_data_offset = off + data_start;
    m_data_size = m_total_size - data_start;
}


}
