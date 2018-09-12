/****************************************************************************
  Copyright (c) 2018 libo All rights reserved.
  
  losemymind.libo@gmail.com

****************************************************************************/
#ifndef NETLITE_ADDRESS_V4_HPP
#define NETLITE_ADDRESS_V4_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <string>
#include <iosfwd>
#include <array>
#include "NetLite/config.hpp"
#include "NetLite/net_error_code.hpp"
#include "NetLite/socket_types.hpp"

namespace NetLite {
namespace ip{

/**
 * Implements IP version 4 style addresses.
 * The NetLite::ip::address_v4 class provides the ability to use and
 * manipulate IP version 4 addresses.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Unsafe.
 */
class address_v4
{
public:
    /// The type used to represent an address as an unsigned integer.
    typedef uint_least32_t uint_type;

    /**
     * The type used to represent an address as an array of bytes.
     * @note This type is defined in terms of the C++0x template @c std::array
     * when it is available. Otherwise, it uses @c boost:array.
     */
    typedef std::array<unsigned char, 4> bytes_type;

    /// Default constructor.
    address_v4()
    {
        addr_.s_addr = 0;
    }

    /// Construct an address from raw bytes.
    NETWORK_API explicit address_v4(const bytes_type& bytes);

    /// Construct an address from an unsigned integer in host byte order.
    NETWORK_API explicit address_v4(uint_type addr);

    /// Copy constructor.
    address_v4(const address_v4& other)
        : addr_(other.addr_)
    {
    }

    /// Move constructor.
    address_v4(address_v4&& other)
        : addr_(other.addr_)
    {
    }

    /// Assign from another address.
    address_v4& operator=(const address_v4& other)
    {
        addr_ = other.addr_;
        return *this;
    }


    /// Move-assign from another address.
    address_v4& operator=(address_v4&& other)
    {
        addr_ = other.addr_;
        return *this;
    }


    /// Get the address in bytes, in network byte order.
    NETWORK_API bytes_type to_bytes() const;

    /// Get the address as an unsigned integer in host byte order
    NETWORK_API uint_type to_uint() const;


    /// Get the address as an unsigned long in host byte order
    NETWORK_API unsigned long to_ulong() const;


    /// Get the address as a string in dotted decimal format.
    NETWORK_API std::string to_string() const;


    /// (Deprecated: Use other overload.) Get the address as a string in dotted
    /// decimal format.
    NETWORK_API std::string to_string(std::error_code& ec) const;

    /// (Deprecated: Use make_address_v4().) Create an address from an IP address
    /// string in dotted decimal form.
    NETWORK_API static address_v4 from_string(const char* str);

    /// (Deprecated: Use make_address_v4().) Create an address from an IP address
    /// string in dotted decimal form.
    NETWORK_API static address_v4 from_string(const char* str, std::error_code& ec);

    /// (Deprecated: Use make_address_v4().) Create an address from an IP address
    /// string in dotted decimal form.
    NETWORK_API static address_v4 from_string(const std::string& str);

    /// (Deprecated: Use make_address_v4().) Create an address from an IP address
    /// string in dotted decimal form.
    NETWORK_API static address_v4 from_string(const std::string& str, std::error_code& ec);


    /// Determine whether the address is a loopback address.
    NETWORK_API bool is_loopback() const;

    /// Determine whether the address is unspecified.
    NETWORK_API bool is_unspecified() const;

    /// (Deprecated: Use network_v4 class.) Determine whether the address is a
    /// class A address.
    NETWORK_API bool is_class_a() const;

    /// (Deprecated: Use network_v4 class.) Determine whether the address is a
    /// class B address.
    NETWORK_API bool is_class_b() const;

    /// (Deprecated: Use network_v4 class.) Determine whether the address is a
    /// class C address.
    NETWORK_API bool is_class_c() const;


    /// Determine whether the address is a multicast address.
    NETWORK_API bool is_multicast() const;

    /// Compare two addresses for equality.
    friend bool operator==(const address_v4& a1, const address_v4& a2)
    {
        return a1.addr_.s_addr == a2.addr_.s_addr;
    }

    /// Compare two addresses for inequality.
    friend bool operator!=(const address_v4& a1, const address_v4& a2)
    {
        return a1.addr_.s_addr != a2.addr_.s_addr;
    }

    /// Compare addresses for ordering.
    friend bool operator<(const address_v4& a1, const address_v4& a2)
    {
        return a1.to_uint() < a2.to_uint();
    }

    /// Compare addresses for ordering.
    friend bool operator>(const address_v4& a1, const address_v4& a2)
    {
        return a1.to_uint() > a2.to_uint();
    }

    /// Compare addresses for ordering.
    friend bool operator<=(const address_v4& a1, const address_v4& a2)
    {
        return a1.to_uint() <= a2.to_uint();
    }

    /// Compare addresses for ordering.
    friend bool operator>=(const address_v4& a1, const address_v4& a2)
    {
        return a1.to_uint() >= a2.to_uint();
    }

    /// Obtain an address object that represents any address.
    static address_v4 any()
    {
        return address_v4();
    }

    /// Obtain an address object that represents the loopback address.
    static address_v4 loopback()
    {
        return address_v4(0x7F000001);
    }

    /// Obtain an address object that represents the broadcast address.
    static address_v4 broadcast()
    {
        return address_v4(0xFFFFFFFF);
    }

    /// (Deprecated: Use network_v4 class.) Obtain an address object that
    /// represents the broadcast address that corresponds to the specified
    /// address and netmask.
    NETWORK_API static address_v4 broadcast(const address_v4& addr, const address_v4& mask);

    /// (Deprecated: Use network_v4 class.) Obtain the netmask that corresponds
    /// to the address, based on its address class.
    NETWORK_API static address_v4 netmask(const address_v4& addr);


private:
    // The underlying IPv4 address.
    NetLite::in4_addr_type addr_;
};

/// Create an IPv4 address from raw bytes in network order.
/**
* @relates address_v4
*/
inline  address_v4 make_address_v4(const address_v4::bytes_type& bytes)
{
    return address_v4(bytes);
}

/**
 * Create an IPv4 address from an unsigned integer in host byte order.
 * @relates address_v4
 */
inline  address_v4 make_address_v4(address_v4::uint_type addr)
{
    return address_v4(addr);
}

/**
 * Create an IPv4 address from an IP address string in dotted decimal form.
 * @relates address_v4
 */
NETWORK_API address_v4 make_address_v4(const char* str);

/**
 * Create an IPv4 address from an IP address string in dotted decimal form.
 * @relates address_v4
 */
NETWORK_API address_v4 make_address_v4(const char* str, std::error_code& ec);

/**
 * Create an IPv4 address from an IP address string in dotted decimal form.
 * @relates address_v4
 */
NETWORK_API address_v4 make_address_v4(const std::string& str);

/**
 * Create an IPv4 address from an IP address string in dotted decimal form.
 * @relates address_v4
 */
NETWORK_API address_v4 make_address_v4(const std::string& str, std::error_code& ec);

/**
 * Output an address as a string.
 * Used to output a human-readable string for a specified address.
 *
 * @param os The output stream to which the string will be written.
 *
 * @param addr The address to be written.
 *
 * @return The output stream.
 *
 * @relates NetLite::ip::address_v4
 */
template <typename Elem, typename Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& os, const address_v4& addr)
{
    return os << addr.to_string().c_str();
}

} // namespace ip
} // namespace NetLite

#include "NetLite/ip/address_v4.ipp"


#endif // END OF NETLITE_ADDRESS_V4_HPP
