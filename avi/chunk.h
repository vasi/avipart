#ifndef __AVI_CHUNK_H__
#define __AVI_CHUNK_H__

#include "avi/chunk_base.h"

namespace avi {

class chunk : public chunk_base {
public:
    chunk(const mmap& map, offset off);
};

}

#endif
