namespace avi {

template <typename T> T mmap::at(offset off) const {
    return *mem<T>(off);
}

template <typename T> const T* mmap::mem(offset off) const {
    return reinterpret_cast<const T*>(cdata(off, sizeof(T)));
}    

}
