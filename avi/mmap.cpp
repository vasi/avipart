#include "avi/mmap.h"

#include "format.h"
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

namespace avi {

mmap::mmap(const std::string& name) {
    mmap::filedesc fd = open(name.c_str(), O_RDONLY);
    if (fd == -1) fmt::perrorf<std::invalid_argument>(_("Can't open avi "
        "file %s"), name.c_str());
    
    try {
        init(fd);
    } catch (const std::exception& e) {
        close(fd);
        throw;
    }
}

mmap::mmap(mmap::filedesc fd) {
    init(fd);
}

void mmap::init(mmap::filedesc fd) {
    m_fd = fd;
    
    struct stat sb;
    if(fstat(fd, &sb)) fmt::perrorf<std::runtime_error>(_("Can't stat avi"
        " file descriptor"));
    m_len = sb.st_size;
    
    m_addr = ::mmap(0, m_len, PROT_READ, MAP_FILE, m_fd, 0);
    if (m_addr == (void*)-1) fmt::perrorf<std::runtime_error>(
        _("Can't mmap avi"));
}

mmap::~mmap() {
    if (m_addr > 0) munmap(m_addr, m_len);
    if (m_fd >= 0) close(m_fd);
}

void mmap::check(offset off, size_t sz) const {
    if (off < 0 || off + sz > m_len)
        throw(std::out_of_range(_("out of range in avi")));
}

const void *mmap::data(offset off, size_t sz) const {
    return cdata(off, sz);
}

const char *mmap::cdata(offset off, size_t sz) const {
    check(off, sz);
    return reinterpret_cast<char*>(m_addr) + off;
}

}
