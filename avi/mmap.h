#ifndef __AVI_MMAP_H__
#define __AVI_MMAP_H__

#include "avi/common.h"
#include <cstddef>
#include <string>

namespace avi {

class mmap {
    typedef int filedesc;
    
    filedesc m_fd;
    size_t m_len;
    void *m_addr;
    
    void init(filedesc fd);
    const char *cdata(offset off, size_t sz) const;
    
    // Don't copy
    const mmap& operator=(const mmap& other);
    mmap(const mmap& other);
    
public:
    mmap(const std::string& name);
    mmap(filedesc fd);
    virtual ~mmap();
    
    size_t size() const;
    
    void check(offset off, size_t sz) const;
    const void *data(offset off, size_t sz) const;
    
    template <typename T> T at(offset off) const;
    template <typename T> const T* mem(offset off) const;
    
    bool operator==(const mmap& other) const;
};


}

#include "avi/mmap.tcc"

#endif
