#include "avi/list_chunk.h"

#include "avi/internal.h"

namespace avi {

struct mem_chunk_list : public mem_chunk_header {
    id list_id;
    char data[];
};


list_chunk::list_chunk(const mmap& map, offset off) : chunk_base(map) {
    const mem_chunk_list *mem = m_map.mem<mem_chunk_list>(off);
    init(off, mem, offsetof_obj(*mem, data));
    m_list_id = mem->list_id;
}

id list_chunk::list_id() const {
    return m_list_id;
}

const mmap& list_chunk::map() const {
    return m_map;
}

offset list_chunk::list_data_offset() const {
    return m_data_offset;
}

size_t list_chunk::list_data_size() const {
    return data_size();
}

const id list_chunk::ID('LIST');

}
