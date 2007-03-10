#include "avi/file.h"

#include "avi/list_chunk.h"
#include "format.h"

namespace avi {

const id file::ID('RIFF');
const id file::LIST_ID('AVI ');

file::file(const std::string& name) : m_map(name) {
    list_chunk ch(m_map, 0);
    if (ch.chunk_id() != file::ID || ch.list_id() != file::LIST_ID)
        fmt::throwf<std::invalid_argument>(_("'%s' doesn't look like an AVI "
            "file"), name.c_str());
}

const mmap& file::map() const {
    return m_map;
}

offset file::list_data_offset() const {
    return 0;
}

size_t file::list_data_size() const {
    return m_map.size();
}


}
