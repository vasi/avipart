#include "endian.h"

#ifdef __APPLE__
	#include <libkern/OSByteOrder.h>
#else
	#error "Don't know how to swap endianness"
#endif

namespace endian {

#ifdef __APPLE__
    
uint32_t fromLE32(uint32_t i) {
    return OSSwapLittleToHostInt32(i);
}

uint32_t toLE32(uint32_t i) {
    return OSSwapHostToLittleInt32(i);
}
    
#endif

}
