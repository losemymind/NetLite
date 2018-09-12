
#ifndef NETLITE_ADDRESS_V4_RANGE_HPP
#define NETLITE_ADDRESS_V4_RANGE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "NetLite/ip/address_v4_iterator.hpp"

namespace NetLite {
namespace ip{

template <typename> 
class basic_address_range;

/**
 * Represents a range of IPv4 addresses.
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Unsafe.
 */
template <> 
class basic_address_range<address_v4>
{
public:
    /// The type of an iterator that points into the range.
    typedef basic_address_iterator<address_v4> iterator;

    /// Construct an empty range.
    basic_address_range() _NOEXCEPT
        : begin_(address_v4())
        , end_(address_v4())
    {
    }

    /// Construct an range that represents the given range of addresses.
    explicit basic_address_range(const iterator& first, const iterator& last) _NOEXCEPT
        : begin_(first)
        , end_(last)
    {
    }

    /// Copy constructor.
    basic_address_range(const basic_address_range& other) _NOEXCEPT
        : begin_(other.begin_)
        , end_(other.end_)
    {
    }

    /// Move constructor.
    basic_address_range(basic_address_range&& other) _NOEXCEPT
        : begin_(static_cast<iterator&&>(other.begin_))
        , end_(static_cast<iterator&&>(other.end_))
    {
    }

    /// Assignment operator.
    basic_address_range& operator=(const basic_address_range& other) _NOEXCEPT
    {
        begin_ = other.begin_;
        end_ = other.end_;
        return *this;
    }

    /// Move assignment operator.
     basic_address_range& operator=(basic_address_range&& other) _NOEXCEPT
    {
        begin_ = static_cast<iterator&&>(other.begin_);
        end_ = static_cast<iterator&&>(other.end_);
        return *this;
    }


    /// Obtain an iterator that points to the start of the range.
    iterator begin() const _NOEXCEPT
    {
        return begin_;
    }

    /// Obtain an iterator that points to the end of the range.
    iterator end() const _NOEXCEPT
    {
        return end_;
    }

    /// Determine whether the range is empty.
    bool empty() const _NOEXCEPT
    {
        return size() == 0;
    }

    /// Return the size of the range.
    std::size_t size() const _NOEXCEPT
    {
        return end_->to_uint() - begin_->to_uint();
    }

    /// Find an address in the range.
    iterator find(const address_v4& addr) const _NOEXCEPT
    {
        return addr >= *begin_ && addr < *end_ ? iterator(addr) : end_;
    }

private:
    iterator begin_;
    iterator end_;
};

/// Represents a range of IPv4 addresses.
typedef basic_address_range<address_v4> address_v4_range;

} // namespace ip
} // namespace NetLite

#endif // END OF NETLITE_ADDRESS_V4_RANGE_HPP