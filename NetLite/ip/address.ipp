
#ifndef NETLITE_ADDRESS_IPP
#define NETLITE_ADDRESS_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "NetLite/ip/address.hpp"
#include "NetLite/ip/bad_address_cast.hpp"
namespace NetLite{
namespace ip{

address::address()
    : type_(ipv4)
    , ipv4_address_()
    , ipv6_address_()
{
}

address::address(const ip::address_v4& ipv4_address)
    : type_(ipv4)
    , ipv4_address_(ipv4_address)
    , ipv6_address_()
{
}

address::address(const ip::address_v6& ipv6_address)
    : type_(ipv6)
    , ipv4_address_()
    , ipv6_address_(ipv6_address)
{
}

address::address(const address& other)
    : type_(other.type_)
    , ipv4_address_(other.ipv4_address_)
    , ipv6_address_(other.ipv6_address_)
{
}


address::address(address&& other)
    : type_(other.type_)
    , ipv4_address_(other.ipv4_address_)
    , ipv6_address_(other.ipv6_address_)
{
}


address& address::operator=(const address& other)
{
    type_ = other.type_;
    ipv4_address_ = other.ipv4_address_;
    ipv6_address_ = other.ipv6_address_;
    return *this;
}


address& address::operator=(address&& other)
{
    type_ = other.type_;
    ipv4_address_ = other.ipv4_address_;
    ipv6_address_ = other.ipv6_address_;
    return *this;
}


address& address::operator=(const ip::address_v4& ipv4_address)
{
    type_ = ipv4;
    ipv4_address_ = ipv4_address;
    ipv6_address_ = ip::address_v6();
    return *this;
}

address& address::operator=(const ip::address_v6& ipv6_address)
{
    type_ = ipv6;
    ipv4_address_ = ip::address_v4();
    ipv6_address_ = ipv6_address;
    return *this;
}

ip::address_v4 address::to_v4() const
{
    if (type_ != ipv4)
    {
        bad_address_cast ex;
        throw ex;
    }
    return ipv4_address_;
}

ip::address_v6 address::to_v6() const
{
    if (type_ != ipv6)
    {
        bad_address_cast ex;
        throw ex;
    }
    return ipv6_address_;
}

std::string address::to_string() const
{
    if (type_ == ipv6)
        return ipv6_address_.to_string();
    return ipv4_address_.to_string();
}


std::string address::to_string(std::error_code& ec) const
{
    if (type_ == ipv6)
        return ipv6_address_.to_string(ec);
    return ipv4_address_.to_string(ec);
}

address address::from_string(const char* str)
{
    return ip::make_address(str);
}

address address::from_string(const char* str, std::error_code& ec)
{
    return ip::make_address(str, ec);
}

address address::from_string(const std::string& str)
{
    return ip::make_address(str);
}

address address::from_string(const std::string& str, std::error_code& ec)
{
    return ip::make_address(str, ec);
}

bool address::is_loopback() const
{
    return (type_ == ipv4)
        ? ipv4_address_.is_loopback()
        : ipv6_address_.is_loopback();
}

bool address::is_unspecified() const
{
    return (type_ == ipv4)
        ? ipv4_address_.is_unspecified()
        : ipv6_address_.is_unspecified();
}

bool address::is_multicast() const
{
    return (type_ == ipv4)
        ? ipv4_address_.is_multicast()
        : ipv6_address_.is_multicast();
}

bool operator==(const address& a1, const address& a2)
{
    if (a1.type_ != a2.type_)
        return false;
    if (a1.type_ == address::ipv6)
        return a1.ipv6_address_ == a2.ipv6_address_;
    return a1.ipv4_address_ == a2.ipv4_address_;
}

bool operator<(const address& a1, const address& a2)
{
    if (a1.type_ < a2.type_)
        return true;
    if (a1.type_ > a2.type_)
        return false;
    if (a1.type_ == address::ipv6)
        return a1.ipv6_address_ < a2.ipv6_address_;
    return a1.ipv4_address_ < a2.ipv4_address_;
}

address make_address(const char* str)
{
    std::error_code ec;
    address addr = make_address(str, ec);
    if (ec) throw ec;
    return addr;
}

address make_address(const char* str, std::error_code& ec)
{
    ip::address_v6 ipv6_address = ip::make_address_v6(str, ec);
    if (!ec)
        return address(ipv6_address);

    ip::address_v4 ipv4_address = ip::make_address_v4(str, ec);
    if (!ec)
        return address(ipv4_address);

    return address();
}

address make_address(const std::string& str)
{
    return make_address(str.c_str());
}

address make_address(const std::string& str, std::error_code& ec)
{
    return make_address(str.c_str(), ec);
}

} // namespace ip
} // namespace NetLite

#endif // END OF NETLITE_ADDRESS_IPP
