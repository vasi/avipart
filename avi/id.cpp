#include "avi/id.h"

#include "format.h"

namespace avi {

const id::type id::DAMAGED_ID_VALUE = 0xFFFFFFFF;

id::id(type i) : m_val(i) { }

id::type id::value() const {
    return m_val;
}

std::string id::string() const {
    return fmt::stringf("%.4s", chars());
}

const char *id::chars() const {
    return reinterpret_cast<const char *>(&m_val);
}

bool id::operator==(const id& other) const {
    return m_val == other.m_val;
}

bool id::operator!=(const id& other) const {
    return !(*this == other);
}

}
