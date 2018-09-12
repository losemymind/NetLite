
#ifndef NETLITE_ADDRESS_V4_IPP
#define NETLITE_ADDRESS_V4_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <climits>
#include <limits>
#include <stdexcept>
#include "NetLite/ip/address_v4.hpp"
#include "NetLite/socket_ops.hpp"

namespace NetLite{
namespace ip{

address_v4::address_v4(const address_v4::bytes_type& bytes)
{
#if UCHAR_MAX > 0xFF
    if (bytes[0] > 0xFF || bytes[1] > 0xFF
        || bytes[2] > 0xFF || bytes[3] > 0xFF)
    {
        std::out_of_range ex("address_v4 from bytes_type");
        throw ex;
    }
#endif // UCHAR_MAX > 0xFF

    using namespace std; // For memcpy.
    memcpy(&addr_.s_addr, bytes.data(), 4);
}

address_v4::address_v4(address_v4::uint_type addr)
{
    if ((std::numeric_limits<uint_type>::max)() > 0xFFFFFFFF)
    {
        std::out_of_range ex("address_v4 from unsigned integer");
        throw(ex);
    }
    addr_.s_addr = socket_ops::host_to_network_long(static_cast<u_long_type>(addr));
}

address_v4::bytes_type address_v4::to_bytes() const
{
    using namespace std; // For memcpy.
    bytes_type bytes;
    memcpy(bytes.data(), &addr_.s_addr, 4);
    return bytes;
}

address_v4::uint_type address_v4::to_uint() const
{
    return socket_ops::network_to_host_long(addr_.s_addr);
}


unsigned long address_v4::to_ulong() const
{
    return socket_ops::network_to_host_long(addr_.s_addr);
}


std::string address_v4::to_string() const
{
    std::error_code ec;
    char addr_str[max_addr_v4_str_len];
    const char* addr = socket_ops::inet_ntop(NET_OS_DEF(AF_INET), &addr_, addr_str, max_addr_v4_str_len, 0, ec);
    if (addr == 0)
        throw(ec);
    return addr;
}


std::string address_v4::to_string(std::error_code& ec) const
{
    char addr_str[max_addr_v4_str_len];
    const char* addr = socket_ops::inet_ntop(NET_OS_DEF(AF_INET), &addr_, addr_str, max_addr_v4_str_len, 0, ec);
    if (addr == 0)
        return std::string();
    return addr;
}

inline address_v4 address_v4::from_string(const char* str)
{
    return ip::make_address_v4(str);
}

inline address_v4 address_v4::from_string(const char* str, std::error_code& ec)
{
    return ip::make_address_v4(str, ec);
}

inline address_v4 address_v4::from_string(const std::string& str)
{
    return ip::make_address_v4(str);
}

inline address_v4 address_v4::from_string(const std::string& str, std::error_code& ec)
{
    return ip::make_address_v4(str, ec);
}

bool address_v4::is_loopback() const
{
    return (to_uint() & 0xFF000000) == 0x7F000000;
}

bool address_v4::is_unspecified() const
{
    return to_uint() == 0;
}

bool address_v4::is_class_a() const
{
    return (to_uint() & 0x80000000) == 0;
}

bool address_v4::is_class_b() const
{
    return (to_uint() & 0xC0000000) == 0x80000000;
}

bool address_v4::is_class_c() const
{
    return (to_uint() & 0xE0000000) == 0xC0000000;
}

bool address_v4::is_multicast() const
{
    return (to_uint() & 0xF0000000) == 0xE0000000;
}

address_v4 address_v4::broadcast(const address_v4& addr, const address_v4& mask)
{
    return address_v4(addr.to_uint() | (mask.to_uint() ^ 0xFFFFFFFF));
}

address_v4 address_v4::netmask(const address_v4& addr)
{
    if (addr.is_class_a())
        return address_v4(0xFF000000);
    if (addr.is_class_b())
        return address_v4(0xFFFF0000);
    if (addr.is_class_c())
        return address_v4(0xFFFFFF00);
    return address_v4(0xFFFFFFFF);
}

address_v4 make_address_v4(const char* str)
{
    std::error_code ec;
    address_v4 addr = make_address_v4(str, ec);
    if(ec) throw ec;
    return addr;
}

address_v4 make_address_v4(const char* str, std::error_code& ec)
{
    address_v4::bytes_type bytes;
    if (socket_ops::inet_pton(NET_OS_DEF(AF_INET), str, &bytes, 0, ec) <= 0)
        return address_v4();
    return address_v4(bytes);
}

address_v4 make_address_v4(const std::string& str)
{
    return make_address_v4(str.c_str());
}

address_v4 make_address_v4( const std::string& str, std::error_code& ec)
{
    return make_address_v4(str.c_str(), ec);
}

} // namespace ip
} // namespace NetLite


#endif // END OF NETLITE_ADDRESS_V4_IPP
