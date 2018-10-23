/****************************************************************************
  Copyright (c) 2018 libo All rights reserved.
  
  losemymind.libo@gmail.com

****************************************************************************/
#ifndef NETLITE_MUTABLEBUF_HPP
#define NETLITE_MUTABLEBUF_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <string>
#include <vector>
#include <memory>

namespace NetLite {
/**
    * Holds a buffer that can be modified.
    * The mutable_buffer class provides a safe representation of a buffer that can
    * be modified. It does not own the underlying data, and so is cheap to copy or
    * assign.
    *
    * @par Accessing Buffer Contents
    *
    * The contents of a buffer may be accessed using the @c data() and @c size()
    * member functions:
    *
    * @code mutable_buffer b1 = ...;
    * std::_sizet s1 = b1.size();
    * unsigned char* p1 = static_cast<unsigned char*>(b1.data());
    * @endcode
    *
    * The @c data() member function permits violations of type safety, so uses of
    * it in application code should be carefully considered.
    */
class basic_mutablebuf
{
public:
    using _Ty = uint8_t;
    using _Myt = basic_mutablebuf;
    using _Ptr = std::shared_ptr<_Ty>;

    /// Construct an empty buffer.
    basic_mutablebuf()
        : memory_holder(nullptr)
        , memory_length(0)
    {
    }

    basic_mutablebuf(const _Myt& other)
        : memory_holder(other.memory_holder)
        , memory_length(other.memory_length)
    {

    }

    basic_mutablebuf(_Myt&& other)
        : memory_holder(std::move(other.memory_holder))
        , memory_length(other.memory_length)
    {
        other.memory_length = 0;
    }

    /// Construct a buffer to represent a given memory range.
    basic_mutablebuf(uint8_t* inData, std::size_t length, bool need_del = false)
    {
        assign(inData, length, need_del);
    }

    /// Construct a buffer to represent a given memory range.
    basic_mutablebuf(char* inData, std::size_t length, bool need_del = false)
    {
        assign(reinterpret_cast<uint8_t*>(inData), length, need_del);
    }

    /// Construct a buffer to represent a given memory range.
    basic_mutablebuf(void* inData, std::size_t length, bool need_del = false)
    {
        assign(reinterpret_cast<uint8_t*>(inData), length, need_del);
    }

    /// Assignment operator
    basic_mutablebuf& operator= (const _Myt& other)
    {
        memory_length = other.memory_length;
        memory_holder = other.memory_holder;
        return (*this);
    }

    basic_mutablebuf& operator= (_Myt&& other)
    {
        memory_length = other.memory_length;
        memory_holder = std::move(other.memory_holder);
        other.memory_length = 0;
        return (*this);
    }

    void* data() const
    {
        return memory_holder.get();
    }

    /// Get the size of the memory range.
    size_t size() const
    {
        return memory_length;
    }

    const char* c_str()const
    {
        return reinterpret_cast<const char*>(data());
    }

    char* str()
    {
        return reinterpret_cast<char*>(data());
    }

    bool empty()
    {
        return !memory_holder || memory_length == 0;
    }


    void assign(uint8_t* inData, std::size_t length, bool need_del = false)
    {
        if (need_del)
        {
            memory_holder = _Ptr(inData);
        }
        else
        {
            memory_holder = _Ptr(inData, [&](_Ty* /*ptr*/) {/*do nothing...*/});
        }
        memory_length = length;
    }

    ~basic_mutablebuf()
    {

    }
protected:
    _Ptr            memory_holder;
    size_t          memory_length;
};

typedef basic_mutablebuf mutablebuf;


inline mutablebuf make_mutablebuf(std::vector<char>& buffers)
{
    return mutablebuf(buffers.data(), buffers.size());
}

inline mutablebuf make_mutablebuf(std::vector<unsigned char>& buffers)
{
    return mutablebuf(buffers.data(), buffers.size());
}

inline mutablebuf make_mutablebuf(std::basic_string<char>& buffers)
{
    return mutablebuf(&(buffers[0]), buffers.size());
}

inline mutablebuf make_mutablebuf(std::basic_string<unsigned char>& buffers)
{
    return mutablebuf(&(buffers[0]), buffers.size());
}


/**
    * Holds a buffer that cannot be modified.
    * The basic_constbuf class provides a safe representation of a buffer that cannot
    * be modified. It does not own the underlying data, and so is cheap to copy or
    * assign.
    *
    * @code basic_constbuf b1 = ...;
    * std::size_t s1 = b1.size();
    * const unsigned char* p1 = static_cast<const unsigned char*>(b1.data());
    * @endcode
    *
    * The data() function permits violations of type safety, so
    * uses of it in application code should be carefully considered.
    */
class basic_constbuf
{
public:

    /// Construct an empty buffer.
    basic_constbuf()
        : _data(nullptr)
        , _size(0)
    {
    }


    basic_constbuf(const basic_constbuf& other)
    {
        copy(other);
    }

    basic_constbuf(basic_constbuf&& other)
    {
        move(std::forward<basic_constbuf&&>(other));
    }

    basic_constbuf(const mutablebuf& mutableBuf)
    {
        this->_size = mutableBuf.size();
        this->_data = mutableBuf.data();
    }

    /// Construct a buffer to represent a given memory range.
    basic_constbuf(const uint8_t* inData, std::size_t length)
        : _data(inData)
        , _size(length)
    {
    }

    /// Construct a buffer to represent a given memory range.
    basic_constbuf(const char* inData, std::size_t length)
        : _data(inData)
        , _size(length)
    {
    }

    /// Construct a buffer to represent a given memory range.
    basic_constbuf(const void* inData, std::size_t length)
        : _data(inData)
        , _size(length)
    {
    }

    // Assignment operator
    basic_constbuf& operator= (const basic_constbuf& other)
    {
        if (this != &other)
        {
            copy(other);
        }
        return *this;
    }

    basic_constbuf& operator= (basic_constbuf&& other)
    {
        if (this != &other)
        {
            move(std::forward<basic_constbuf&&>(other));
        }
        return *this;
    }

    /// Get a pointer to the beginning of the memory range.
    const void* data() const
    {
        return _data;
    }

    /// Get the size of the memory range.
    std::size_t size() const
    {
        return _size;
    }

    const char* c_str()const
    {
        return reinterpret_cast<const char*>(_data);
    }

    const uint8_t* uc_str()
    {
        return reinterpret_cast<const uint8_t*>(_data);
    }

    void assign(void* inData, std::size_t length)
    {
        clear();
        _data = inData;
        _size = length;
    }

    void clear()
    {
        _data = nullptr;
        _size = 0;
    }

    bool empty()
    {
        return (_data == nullptr || _size == 0);
    }

    ~basic_constbuf()
    {
        clear();
    }

private:
    void copy(const basic_constbuf& other)
    {
        this->_size = other._size;
        this->_data = other._data;
    }

    void move(basic_constbuf&& other)
    {
        this->_data = other._data;
        this->_size = other._size;
        other._data = nullptr;
        other._size = 0;
    }

private:
    const void* _data;
    std::size_t _size;
};

typedef basic_constbuf constbuf;

inline constbuf make_constbuf(const std::vector<char>& buffers)
{
    return constbuf(&(buffers[0]), buffers.size());
}

inline constbuf make_constbuf(const std::vector<unsigned char>& buffers)
{
    return constbuf(&(buffers[0]), buffers.size());
}

inline constbuf make_constbuf(const std::basic_string<char>& buffers)
{
    return constbuf(&(buffers[0]), buffers.size());
}

inline constbuf make_constbuf(const std::basic_string<unsigned char>& buffers)
{
    return constbuf(&(buffers[0]), buffers.size());
}

inline constbuf make_constbuf(const mutablebuf& buffers)
{
    return constbuf(buffers.data(), buffers.size());
}

} // namespace NetLite
#endif // END OF NETLITE_MUTABLEBUF_HPP
