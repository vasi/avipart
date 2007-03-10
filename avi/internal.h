#ifndef __AVI_INTERNAL_H__

namespace avi {

// Yech, GCC doesn't like offsetof for non-PODs. So define a runtime version
// that works on objects instead.
#define offsetof_obj(obj, member) ((char*)&(obj).member - (char*)&(obj))

}

#endif
