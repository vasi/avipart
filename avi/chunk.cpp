#include "avi/chunk.h"

#include "avi/internal.h"

namespace avi {

struct mem_chunk : public mem_chunk_header {
    char data[];
};

chunk::chunk(const mmap& map, offset off) : chunk_base(map) {
    const mem_chunk *mem = m_map.mem<mem_chunk>(off);
    init(off, m_map.mem<mem_chunk>(off), offsetof_obj(*mem, data));
}

}
