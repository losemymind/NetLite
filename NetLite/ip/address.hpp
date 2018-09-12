/****************************************************************************
  Copyright (c) 2018 libo All rights reserved.
  
  losemymind.libo@gmail.com

****************************************************************************/
#ifndef NETLITE_ADDRESS_HPP
#define NETLITE_ADDRESS_HPP

#include <string>
#include <iosfwd>
#include "NetLite/config.hpp"
#include "NetLite/net_error_code.hpp"
#include "NetLite/ip/address_v4.hpp"
#include "NetLite/ip/address_v6.hpp"
#include "NetLite/ip/bad_address_cast.hpp"

namespace NetLite{
namespace ip{
/**
 * Implements version-independent IP addresses.
 * The ip::address class provides the ability to use either IP
 * version 4 or version 6 addresses.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Unsafe.
 */
class address
{
public:
    /// Default constructor.
    NETWORK_API address();

    /// Construct an address from an IPv4 address.
    NETWORK_API address(const ip::address_v4& ipv4_address);

    /// Construct an address from an IPv6 address.
    NETWORK_API address(const ip::address_v6& ipv6_address);

    /// Copy constructor.
    NETWORK_API address(const address& other);

    /// Move constructor.
    NETWORK_API address(address&& other);

    /// Assign from another address.
    NETWORK_API address& operator=(const address& other);

    /// Move-assign from another address.
    NETWORK_API address& operator=(address&& other);

    /// Assign from an IPv4 address.
    NETWORK_API address& operator=(const ip::address_v4& ipv4_address);

    /// Assign from an IPv6 address.
    NETWORK_API address& operator=(const ip::address_v6& ipv6_address);

    /// Get whether the address is an IP version 4 address.
    bool is_v4() const
    {
        return type_ == ipv4;
    }

    /// Get whether the address is an IP version 6 address.
    bool is_v6() const
    {
        return type_ == ipv6;
    }

    /// Get the address as an IP version 4 address.
    NETWORK_API ip::address_v4 to_v4() const;

    /// Get the address as an IP version 6 address.
    NETWORK_API ip::address_v6 to_v6() const;

    /// Get the address as a string.
    NETWORK_API std::string to_string() const;

    /// (Deprecated: Use other overload.) Get the address as a string.
    NETWORK_API std::string to_string(std::error_code& ec) const;

    /// (Deprecated: Use make_address().) Create an address from an IPv4 address
    /// string in dotted decimal form, or from an IPv6 address in hexadecimal
    /// notation.
    NETWORK_API static address from_string(const char* str);

    /// (Deprecated: Use make_address().) Create an address from an IPv4 address
    /// string in dotted decimal form, or from an IPv6 address in hexadecimal
    /// notation.
    NETWORK_API static address from_string(const char* str, std::error_code& ec);

    /// (Deprecated: Use make_address().) Create an address from an IPv4 address
    /// string in dotted decimal form, or from an IPv6 address in hexadecimal
    /// notation.
    NETWORK_API static address from_string(const std::string& str);

    /// (Deprecated: Use make_address().) Create an address from an IPv4 address
    /// string in dotted decimal form, or from an IPv6 address in hexadecimal
    /// notation.
    NETWORK_API static address from_string(const std::string& str, std::error_code& ec);

    /// Determine whether the address is a loopback address.
    NETWORK_API bool is_loopback() const;

    /// Determine whether the address is unspecified.
    NETWORK_API bool is_unspecified() const;

    /// Determine whether the address is a multicast address.
    NETWORK_API bool is_multicast() const;

    /// Compare two addresses for equality.
    NETWORK_API friend bool operator==(const address& a1, const address& a2);

    /// Compare two addresses for inequality.
    friend bool operator!=(const address& a1, const address& a2)
    {
        return !(a1 == a2);
    }

    /// Compare addresses for ordering.
    NETWORK_API friend bool operator<(const address& a1, const address& a2);

    /// Compare addresses for ordering.
    friend bool operator>(const address& a1, const address& a2)
    {
        return a2 < a1;
    }

    /// Compare addresses for ordering.
    friend bool operator<=(const address& a1, const address& a2)
    {
        return !(a2 < a1);
    }

    /// Compare addresses for ordering.
    friend bool operator>=(const address& a1, const address& a2)
    {
        return !(a1 < a2);
    }

private:
    // The type of the address.
    enum { ipv4, ipv6 } type_;

    // The underlying IPv4 address.
    ip::address_v4 ipv4_address_;

    // The underlying IPv6 address.
    ip::address_v6 ipv6_address_;
};

/**
 * Create an address from an IPv4 address string in dotted decimal form,
 * or from an IPv6 address in hexadecimal notation.
 * @relates address
 */
NETWORK_API address make_address(const char* str);

/**
 * Create an address from an IPv4 address string in dotted decimal form,
 * or from an IPv6 address in hexadecimal notation.
 * @relates address
 */
NETWORK_API address make_address(const char* str, std::error_code& ec);

/**
 * Create an address from an IPv4 address string in dotted decimal form,
 * or from an IPv6 address in hexadecimal notation.
 * @relates address
 */
NETWORK_API address make_address(const std::string& str);

/**
 * Create an address from an IPv4 address string in dotted decimal form,
 * or from an IPv6 address in hexadecimal notation.
 * @relates address
 */
NETWORK_API address make_address(const std::string& str, std::error_code& ec);

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
 * @relates ip::address
 */
template <typename Elem, typename Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& os, const address& addr)
{
    return os << addr.to_string().c_str();
}

} // namespace ip
} // namespace NetLite

#include "NetLite/ip/address.ipp"


#endif // END OF NETLITE_ADDRESS_HPP
