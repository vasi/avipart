#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <cctype>
#include <cerrno>
#include <cstdarg>
#include <cstdio>

#include <algorithm>
#include <stdexcept>
#include <string>
#include <sstream>

#define _(x) x // No translation yet

/***** Endian swapping *****/

namespace endian {
uint32_t fromLE32(uint32_t i);
uint32_t toLE32(uint32_t i);

#ifdef __APPLE__
	#include <libkern/OSByteOrder.h>
    inline uint32_t fromLE32(uint32_t i) {
        return OSSwapLittleToHostInt32(i);
    }
    inline uint32_t toLE32(uint32_t i) {
        return OSSwapHostToLittleInt32(i);
    }
#else
	#error "Don't know how to swap endianness"
#endif
}

/***** String formats *****/

namespace fmt {
std::string vstringf(const char *fmt, std::va_list ap);
std::string stringf(const char *fmt, ...);
template <typename T> void throwf(const char *fmt, ...);
template <typename T> void perror(const char *fmt, ...);

std::string vstringf(const char *fmt, std::va_list ap) {
    char *cstr;
    int err = vasprintf(&cstr, fmt, ap); // not in std!?
    if (err == -1) throw std::bad_alloc();
    
    std::string str(cstr);
    std::free(cstr);
    return str;
}

// Make a string from the format arguments, available as 'var'
#define FMT_STRING(var) \
    std::va_list ap; \
    va_start(ap, fmt); \
    std::string var; \
    try { /* Grrr, C++ needs 'finally' */ \
        var = vstringf(fmt, ap); \
        va_end(ap); \
    } catch (const std::exception& e) { \
        va_end(ap); \
        throw; \
    }

std::string stringf(const char *fmt, ...) {
    FMT_STRING(s)
    return s;
}

template <typename T> void throwf(const char *fmt, ...) {
    FMT_STRING(s)
    throw(T(s));
}

template <typename T> void perrorf(const char *fmt, ...) {
    int saveerr = errno;
    FMT_STRING(s)
    
    static size_t bufsize = 256;
    static char* buf = new char[bufsize];
    
    if (saveerr) {
        do {
            int err = strerror_r(saveerr, buf, bufsize);
            if (err == ERANGE) {
                bufsize *= 2;
                delete[] buf;
                buf = new char[bufsize];
            } else {
                break;
            }
        } while (true);
        s = stringf("%s: %s", s.c_str(), buf);
    }
    throw(T(s));
}

#undef FMT_STRING
} // namespace fmt


/***** AVI classes *****/

namespace avi {

typedef int filedesc;
typedef uint32_t offset;
typedef uint32_t size; // little endian!

class id {
    typedef uint32_t type;
    static const type DAMAGED_ID_VALUE;
    
    type m_val;
    
public:
    id(type i = DAMAGED_ID_VALUE) : m_val(i) { }
    
    type value() const { return m_val; }
    std::string string() const { return fmt::stringf("%.4s", chars()); }
    const char *chars() const { return reinterpret_cast<const char *>(&m_val); }
    
    bool operator==(const id& other) const { return m_val == other.m_val; }
    bool operator!=(const id& other) const { return !(*this == other); }
};

const id::type id::DAMAGED_ID_VALUE = 0xFFFFFFFF;

const id LIST_ID('LIST');
const id AVI_ID('RIFF');
const id AVI_LIST_ID('AVI ');

class mmap {
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
    mmap(filedesc fd) { init(fd); }
    virtual ~mmap();
    
    void check(offset off, size_t sz) const;
    const void *data(offset off, size_t sz) const { return cdata(off, sz); }
    
    template <typename T> T at(offset off) const { return *mem<T>(off); }
    template <typename T> const T* mem(offset off) const;
};

mmap::mmap(const std::string& name) {
    filedesc fd = open(name.c_str(), O_RDONLY);
    if (fd == -1) fmt::perrorf<std::invalid_argument>(_("Can't open avi "
        "file %s"), name.c_str());
    
    try {
        init(fd);
    } catch (const std::exception& e) {
        close(fd);
        throw;
    }
}

void mmap::init(filedesc fd) {
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

const char *mmap::cdata(offset off, size_t sz) const {
    check(off, sz);
    return reinterpret_cast<char*>(m_addr) + off;
}

template <typename T> const T* mmap::mem(offset off) const {
    return reinterpret_cast<const T*>(cdata(off, sizeof(T)));
}    


struct mem_chunk_header {
    id chunk_id;
    size decl_size;
};

class chunk_base { // abstract
protected:
    const mmap& m_map;
    
    id m_chunk_id;
    size_t m_total_size;
    size_t m_data_size;
    offset m_data_offset;
    
    // Call in subclass constructor. Data_start should be the offset from
    // 'off' to the start of the chunk's data.
    void init(offset off, const mem_chunk_header *mem, size_t data_start);
    
public:
    chunk_base(const mmap& map) : m_map(map) { }
    virtual ~chunk_base() = 0;
    
    id chunk_id() const { return m_chunk_id; }
    bool is_list() const;
        
    size_t total_size() const { return m_total_size; }
    size_t data_size() const { return m_data_size; }
    const void *data() const { return m_map.data(m_data_offset, m_data_size); }
};

bool chunk_base::is_list() const {
    return m_chunk_id == LIST_ID || m_chunk_id == AVI_ID;
}

void chunk_base::init(offset off, const mem_chunk_header *mem,
        size_t data_start) {
    m_chunk_id = mem->chunk_id;
    
    size sz = mem->decl_size;
    sz = endian::fromLE32(sz);
    m_total_size = sz + 8;
    
    m_data_offset = off + data_start;
    m_data_size = m_total_size - data_start;
}

chunk_base::~chunk_base() { }


// Yech, GCC doesn't like offsetof for non-PODs. So define a runtime version
// that works on objects instead.
#define offsetof_obj(obj, member) ((char*)&(obj).member - (char*)&(obj))

struct mem_chunk : public mem_chunk_header {
    char data[];
};

class chunk : public chunk_base {
public:
    chunk(const mmap& map, offset off);
};

chunk::chunk(const mmap& map, offset off) : chunk_base(map) {
    const mem_chunk *mem = m_map.mem<mem_chunk>(off);
    init(off, m_map.mem<mem_chunk>(off), offsetof_obj(*mem, data));
}


struct mem_chunk_list : public mem_chunk_header {
    id list_id;
    char data[];
};

class list_chunk : public chunk_base {
    id m_list_id;
    
public:
    static const id ID;
    
    list_chunk(const mmap& map, offset off);
    
    id list_id() const { return m_list_id; }
};

list_chunk::list_chunk(const mmap& map, offset off) : chunk_base(map) {
    const mem_chunk_list *mem = m_map.mem<mem_chunk_list>(off);
    init(off, mem, offsetof_obj(*mem, data));
    m_list_id = mem->list_id;
}

const id list_chunk::ID('LIST');


class file {
    mmap m_map;
    
public:
    static const id ID;
    static const id LIST_ID;
    
    file(const std::string& name);
    virtual ~file() { }
};

file::file(const std::string& name) : m_map(name) {
    list_chunk ch(m_map, 0);
    if (ch.chunk_id() != AVI_ID || ch.list_id() != AVI_LIST_ID)
        fmt::throwf<std::invalid_argument>(_("'%s' doesn't look like an AVI "
            "file"), name.c_str());
}

const id file::ID('RIFF');
const id file::LIST_ID('AVI ');

} // namespace avi

/***** Program logic *****/

static char* progname;

// Convert a string to another type
template <typename T> T convert_string(const std::string& str,
    const std::string& desc = "", bool whole_string = true,
    std::string* o_remain = 0L);

// Get a total byte size from a human size spec (eg: "100M")
// Currently assume only K, M, G are valid suffices, and refer to powers of 1024
template <typename T> T byte_size(const std::string& str);

// Print usage, return error code
int usage(const std::string& err = "");


template <typename T> T convert_string(const std::string& str,
        const std::string& desc, bool whole_string, std::string* o_remain) {
    std::istringstream ss(str);
    T t;
    ss >> t;
    
    if (ss.fail() || (whole_string && !ss.eof())) {
        std::string to = desc.empty() ? ""
            : fmt::stringf(_(" to a %s"), desc.c_str());
        std::string conv = ss.fail() ? _("convert") : _("completely convert"); 
        fmt::throwf<std::invalid_argument>(_("Can't %s '%s'%s"), conv.c_str(),
            str.c_str(), to.c_str());
    }
    
    if (o_remain) ss >> *o_remain;
    return t;
}

template <typename T> T byte_size(const std::string& str) {
    std::string suf;
    T bytes = convert_string<T>(str, "integer", false, &suf);
    
    std::transform(suf.begin(), suf.end(), suf.begin(), &tolower);
    if (suf.size() == 2 && suf.at(1) == 'b') // Allow a B in suffix
        suf.resize(1);
    
    static std::string sufs[] = { "", "k", "m", "g" };
    bool found = false;
    for (size_t i = 0; i < sizeof(sufs)/sizeof(sufs[0]); ++i) {
        if (sufs[i] == suf) {
            found = true;
            break;
        }
        bytes *= 1024;
    }
    if (!found)
        fmt::throwf<std::invalid_argument>(
            _("'%s' doesn't seem to be a byte size"), str.c_str());
    return bytes;
}

int usage(const std::string& err) {
	if (!err.empty()) fprintf(stderr, "%s\n\n", err.c_str());
	fprintf(stderr,
        _("Usage: %s input.avi output.avi PIECE-SIZE START-PIECE\n"), progname);
	fprintf(stderr, _("Extract a part of an avi file.\n"));
	return -1;
}

// NOTE: No character encoding support. Yes, I suck.
int main(int argc, char *argv[]) {
	progname = argv[0];
	
	if (argc == 1) return usage();
	if (argc != 5) return usage(_("Four arguments required"));
	std::string infile = argv[1], outfile = argv[2], psizestr = argv[3],
        pnumstr = argv[4];
    
    try {
        avi::offset offset = byte_size<avi::offset>(psizestr);
        offset *= convert_string<avi::offset>(pnumstr, "integer");
        
        avi::file file(infile);        
    } catch (const std::exception& e) {
        fprintf(stderr, _("Error: %s\n"), e.what());
        return -2;
    }
	
	return 0;
}
