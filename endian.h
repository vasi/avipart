#ifndef __ENDIAN_H__
#define __ENDIAN_H__

#include <stdint.h>

namespace endian {
    uint32_t fromLE32(uint32_t i);
    uint32_t toLE32(uint32_t i);
}

#endif
