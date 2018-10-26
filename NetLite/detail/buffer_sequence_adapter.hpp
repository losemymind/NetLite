
#include <vector>
#include "NetLite/socket_types.hpp"
#include "NetLite/mutablebuf.hpp"
namespace NetLite {

class buffer_sequence_adapter_base
{

#if defined(_WIN32) || defined(__CYGWIN__)
public:
    // The maximum number of buffers to support in a single operation.
    enum { max_buffers = 64 < max_iov_len ? 64 : max_iov_len };

protected:
    typedef WSABUF native_buffer_type;

    template<typename Buffer>
    static void init_native_buffer(WSABUF& buf, const mutablebuf& buffer)
    {
        buf.buf = static_cast<char*>(buffer.data());
        buf.len = static_cast<ULONG>(buffer.size());
    }

    static void init_native_buffer(WSABUF& buf, const constbuf& buffer)
    {
        buf.buf = const_cast<char*>(static_cast<const char*>(buffer.data()));
        buf.len = static_cast<ULONG>(buffer.size());
    }
#else // if(!(defined(_WIN32) || defined(__CYGWIN__)))
public:
    // The maximum number of buffers to support in a single operation.
    enum { max_buffers = 64 < max_iov_len ? 64 : max_iov_len };

protected:
    typedef iovec native_buffer_type;

    static void init_iov_base(void*& base, void* addr)
    {
        base = addr;
    }

    template <typename T>
    static void init_iov_base(T& base, void* addr)
    {
        base = static_cast<T>(addr);
    }

    static void init_native_buffer(iovec& iov, const mutablebuf& buffer)
    {
        init_iov_base(iov.iov_base, buffer.data());
        iov.iov_len = buffer.size();
    }

    static void init_native_buffer(iovec& iov, const constbuf& buffer)
    {
        init_iov_base(iov.iov_base, const_cast<void*>(buffer.data()));
        iov.iov_len = buffer.size();
    }
#endif // defined(_WIN32) || defined(__CYGWIN__)
};

// Helper class to translate buffers into the native buffer representation.
// Buffers type of (std::vector<std::string> or std::vector<std::vector<char|unsigned char>>)
template <typename Buffer, typename Buffers>
class buffer_sequence_adapter : buffer_sequence_adapter_base
{
public:
    explicit buffer_sequence_adapter(const Buffers& buffer_sequence)
        : count_(0), total_buffer_size_(0)
    {
        buffer_sequence_adapter::init(buffer_sequence.begin(), buffer_sequence.end());
    }

    native_buffer_type* buffers()
    {
        return buffers_;
    }

    std::size_t count() const
    {
        return count_;
    }

    std::size_t total_size() const
    {
        return total_buffer_size_;
    }

private:
    template <typename Iterator>
    void init(Iterator begin, Iterator end)
    {
        Iterator iter = begin;
        for (; iter != end && count_ < max_buffers; ++iter, ++count_)
        {
            Buffer buffer = make_buffer<Buffer, decltype((*iter))>::make((*iter));
            init_native_buffer(buffers_[count_], buffer);
            total_buffer_size_ += buffer.size();
        }
    }
    native_buffer_type buffers_[max_buffers];
    std::size_t count_;
    std::size_t total_buffer_size_;
};
} // namespace NetLite