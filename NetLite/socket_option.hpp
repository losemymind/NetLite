
#ifndef NETLITE_SOCKET_OPTION_HPP
#define NETLITE_SOCKET_OPTION_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)


#include <cstddef>
#include <stdexcept>
#include "NetLite/net_error_code.hpp"
#include "NetLite/socket_types.hpp"
#include "NetLite/ip/address.hpp"


namespace NetLite{
namespace socket_option{

// Helper template for implementing boolean-based options.
template <int Level, int Name>
class boolean
{
public:
    // Default constructor.
    boolean()
        : value_(0)
    {
    }

    // Construct with a specific option value.
    explicit boolean(bool v)
        : value_(v ? 1 : 0)
    {
    }

    // Set the current value of the boolean.
    boolean& operator=(bool v)
    {
        value_ = v ? 1 : 0;
        return *this;
    }

    // Get the current value of the boolean.
    bool value() const
    {
        return !!value_;
    }

    // Convert to bool.
    operator bool() const
    {
        return !!value_;
    }

    // Test for false.
    bool operator!() const
    {
        return !value_;
    }

    // Get the level of the socket option.
    template <typename Protocol>
    int level(const Protocol&) const
    {
        return Level;
    }

    // Get the name of the socket option.
    template <typename Protocol>
    int name(const Protocol&) const
    {
        return Name;
    }

    // Get the address of the boolean data.
    template <typename Protocol>
    int* data(const Protocol&)
    {
        return &value_;
    }

    // Get the address of the boolean data.
    template <typename Protocol>
    const int* data(const Protocol&) const
    {
        return &value_;
    }

    // Get the size of the boolean data.
    template <typename Protocol>
    std::size_t size(const Protocol&) const
    {
        return sizeof(value_);
    }

    // Set the size of the boolean data.
    template <typename Protocol>
    void resize(const Protocol&, std::size_t s)
    {
        // On some platforms (e.g. Windows Vista), the getsockopt function will
        // return the size of a boolean socket option as one byte, even though a
        // four byte integer was passed in.
        switch (s)
        {
        case sizeof(char) :
            value_ = *reinterpret_cast<char*>(&value_) ? 1 : 0;
            break;
        case sizeof(value_) :
            break;
        default:
        {
            std::length_error ex("boolean socket option resize");
            throw ex;
        }
        }
    }

private:
    int value_;
};

// Helper template for implementing integer options.
template <int Level, int Name>
class integer
{
public:
    // Default constructor.
    integer()
        : value_(0)
    {
    }

    // Construct with a specific option value.
    explicit integer(int v)
        : value_(v)
    {
    }

    // Set the value of the int option.
    integer& operator=(int v)
    {
        value_ = v;
        return *this;
    }

    // Get the current value of the int option.
    int value() const
    {
        return value_;
    }

    // Get the level of the socket option.
    template <typename Protocol>
    int level(const Protocol&) const
    {
        return Level;
    }

    // Get the name of the socket option.
    template <typename Protocol>
    int name(const Protocol&) const
    {
        return Name;
    }

    // Get the address of the int data.
    template <typename Protocol>
    int* data(const Protocol&)
    {
        return &value_;
    }

    // Get the address of the int data.
    template <typename Protocol>
    const int* data(const Protocol&) const
    {
        return &value_;
    }

    // Get the size of the int data.
    template <typename Protocol>
    std::size_t size(const Protocol&) const
    {
        return sizeof(value_);
    }

    // Set the size of the int data.
    template <typename Protocol>
    void resize(const Protocol&, std::size_t s)
    {
        if (s != sizeof(value_))
        {
            std::length_error ex("integer socket option resize");
            throw ex;
        }
    }

private:
    int value_;
};

// Helper template for implementing linger options.
template <int Level, int Name>
class linger
{
public:
    // Default constructor.
    linger()
    {
        value_.l_onoff = 0;
        value_.l_linger = 0;
    }

    // Construct with specific option values.
    linger(bool e, int t)
    {
        enabled(e);
        timeout(t);
    }

    // Set the value for whether linger is enabled.
    void enabled(bool value)
    {
        value_.l_onoff = value ? 1 : 0;
    }

    // Get the value for whether linger is enabled.
    bool enabled() const
    {
        return value_.l_onoff != 0;
    }

    // Set the value for the linger timeout.
    void timeout(int value)
    {
#if defined(WIN32)
        value_.l_linger = static_cast<u_short>(value);
#else
        value_.l_linger = value;
#endif
    }

    // Get the value for the linger timeout.
    int timeout() const
    {
        return static_cast<int>(value_.l_linger);
    }

    // Get the level of the socket option.
    template <typename Protocol>
    int level(const Protocol&) const
    {
        return Level;
    }

    // Get the name of the socket option.
    template <typename Protocol>
    int name(const Protocol&) const
    {
        return Name;
    }

    // Get the address of the linger data.
    template <typename Protocol>
    NetLite::linger_type* data(const Protocol&)
    {
        return &value_;
    }

    // Get the address of the linger data.
    template <typename Protocol>
    const NetLite::linger_type* data(const Protocol&) const
    {
        return &value_;
    }

    // Get the size of the linger data.
    template <typename Protocol>
    std::size_t size(const Protocol&) const
    {
        return sizeof(value_);
    }

    // Set the size of the int data.
    template <typename Protocol>
    void resize(const Protocol&, std::size_t s)
    {
        if (s != sizeof(value_))
        {
            std::length_error ex("linger socket option resize");
            throw ex;
        }
    }

private:
    NetLite::linger_type value_;
};

// Helper template for implementing multicast enable loopback options.
template <int IPv4_Level, int IPv4_Name, int IPv6_Level, int IPv6_Name>
class multicast_enable_loopback
{
public:
#if defined(__sun) || defined(__osf__)
    typedef unsigned char ipv4_value_type;
    typedef unsigned char ipv6_value_type;
#elif defined(_AIX) || defined(__hpux) || defined(__QNXNTO__) 
    typedef unsigned char ipv4_value_type;
    typedef unsigned int ipv6_value_type;
#else
    typedef int ipv4_value_type;
    typedef int ipv6_value_type;
#endif

    // Default constructor.
    multicast_enable_loopback()
        : ipv4_value_(0),
        ipv6_value_(0)
    {
    }

    // Construct with a specific option value.
    explicit multicast_enable_loopback(bool v)
        : ipv4_value_(v ? 1 : 0),
        ipv6_value_(v ? 1 : 0)
    {
    }

    // Set the value of the boolean.
    multicast_enable_loopback& operator=(bool v)
    {
        ipv4_value_ = v ? 1 : 0;
        ipv6_value_ = v ? 1 : 0;
        return *this;
    }

    // Get the current value of the boolean.
    bool value() const
    {
        return !!ipv4_value_;
    }

    // Convert to bool.
    operator bool() const
    {
        return !!ipv4_value_;
    }

    // Test for false.
    bool operator!() const
    {
        return !ipv4_value_;
    }

    // Get the level of the socket option.
    template <typename Protocol>
    int level(const Protocol& protocol) const
    {
        if (protocol.family() == PF_INET6)
            return IPv6_Level;
        return IPv4_Level;
    }

    // Get the name of the socket option.
    template <typename Protocol>
    int name(const Protocol& protocol) const
    {
        if (protocol.family() == PF_INET6)
            return IPv6_Name;
        return IPv4_Name;
    }

    // Get the address of the boolean data.
    template <typename Protocol>
    void* data(const Protocol& protocol)
    {
        if (protocol.family() == PF_INET6)
            return &ipv6_value_;
        return &ipv4_value_;
    }

    // Get the address of the boolean data.
    template <typename Protocol>
    const void* data(const Protocol& protocol) const
    {
        if (protocol.family() == PF_INET6)
            return &ipv6_value_;
        return &ipv4_value_;
    }

    // Get the size of the boolean data.
    template <typename Protocol>
    std::size_t size(const Protocol& protocol) const
    {
        if (protocol.family() == PF_INET6)
            return sizeof(ipv6_value_);
        return sizeof(ipv4_value_);
    }

    // Set the size of the boolean data.
    template <typename Protocol>
    void resize(const Protocol& protocol, std::size_t s)
    {
        if (protocol.family() == PF_INET6)
        {
            if (s != sizeof(ipv6_value_))
            {
                std::length_error ex("multicast_enable_loopback socket option resize");
                throw_exception(ex);
            }
            ipv4_value_ = ipv6_value_ ? 1 : 0;
        }
        else
        {
            if (s != sizeof(ipv4_value_))
            {
                std::length_error ex("multicast_enable_loopback socket option resize");
                throw_exception(ex);
            }
            ipv6_value_ = ipv4_value_ ? 1 : 0;
        }
    }

private:
    ipv4_value_type ipv4_value_;
    ipv6_value_type ipv6_value_;
};

// Helper template for implementing unicast hops options.
template <int IPv4_Level, int IPv4_Name, int IPv6_Level, int IPv6_Name>
class unicast_hops
{
public:
    // Default constructor.
    unicast_hops()
        : value_(0)
    {
    }

    // Construct with a specific option value.
    explicit unicast_hops(int v)
        : value_(v)
    {
    }

    // Set the value of the option.
    unicast_hops& operator=(int v)
    {
        value_ = v;
        return *this;
    }

    // Get the current value of the option.
    int value() const
    {
        return value_;
    }

    // Get the level of the socket option.
    template <typename Protocol>
    int level(const Protocol& protocol) const
    {
        if (protocol.family() == PF_INET6)
            return IPv6_Level;
        return IPv4_Level;
    }

    // Get the name of the socket option.
    template <typename Protocol>
    int name(const Protocol& protocol) const
    {
        if (protocol.family() == PF_INET6)
            return IPv6_Name;
        return IPv4_Name;
    }

    // Get the address of the data.
    template <typename Protocol>
    int* data(const Protocol&)
    {
        return &value_;
    }

    // Get the address of the data.
    template <typename Protocol>
    const int* data(const Protocol&) const
    {
        return &value_;
    }

    // Get the size of the data.
    template <typename Protocol>
    std::size_t size(const Protocol&) const
    {
        return sizeof(value_);
    }

    // Set the size of the data.
    template <typename Protocol>
    void resize(const Protocol&, std::size_t s)
    {
        if (s != sizeof(value_))
        {
            std::length_error ex("unicast hops socket option resize");
            throw_exception(ex);
        }
#if defined(__hpux)
        if (value_ < 0)
            value_ = value_ & 0xFF;
#endif
    }

private:
    int value_;
};

// Helper template for implementing multicast hops options.
template <int IPv4_Level, int IPv4_Name, int IPv6_Level, int IPv6_Name>
class multicast_hops
{
public:
#if PLATFORM_WINDOWS && defined(UNDER_CE)
    typedef int ipv4_value_type;
#else
    typedef unsigned char ipv4_value_type;
#endif
    typedef int ipv6_value_type;

    // Default constructor.
    multicast_hops()
        : ipv4_value_(0),
        ipv6_value_(0)
    {
    }

    // Construct with a specific option value.
    explicit multicast_hops(int v)
    {
        if (v < 0 || v > 255)
        {
            std::out_of_range ex("multicast hops value out of range");
            throw_exception(ex);
        }
        ipv4_value_ = (ipv4_value_type)v;
        ipv6_value_ = v;
    }

    // Set the value of the option.
    multicast_hops& operator=(int v)
    {
        if (v < 0 || v > 255)
        {
            std::out_of_range ex("multicast hops value out of range");
            throw_exception(ex);
        }
        ipv4_value_ = (ipv4_value_type)v;
        ipv6_value_ = v;
        return *this;
    }

    // Get the current value of the option.
    int value() const
    {
        return ipv6_value_;
    }

    // Get the level of the socket option.
    template <typename Protocol>
    int level(const Protocol& protocol) const
    {
        if (protocol.family() == PF_INET6)
            return IPv6_Level;
        return IPv4_Level;
    }

    // Get the name of the socket option.
    template <typename Protocol>
    int name(const Protocol& protocol) const
    {
        if (protocol.family() == PF_INET6)
            return IPv6_Name;
        return IPv4_Name;
    }

    // Get the address of the data.
    template <typename Protocol>
    void* data(const Protocol& protocol)
    {
        if (protocol.family() == PF_INET6)
            return &ipv6_value_;
        return &ipv4_value_;
    }

    // Get the address of the data.
    template <typename Protocol>
    const void* data(const Protocol& protocol) const
    {
        if (protocol.family() == PF_INET6)
            return &ipv6_value_;
        return &ipv4_value_;
    }

    // Get the size of the data.
    template <typename Protocol>
    std::size_t size(const Protocol& protocol) const
    {
        if (protocol.family() == PF_INET6)
            return sizeof(ipv6_value_);
        return sizeof(ipv4_value_);
    }

    // Set the size of the data.
    template <typename Protocol>
    void resize(const Protocol& protocol, std::size_t s)
    {
        if (protocol.family() == PF_INET6)
        {
            if (s != sizeof(ipv6_value_))
            {
                std::length_error ex("multicast hops socket option resize");
                throw_exception(ex);
            }
            if (ipv6_value_ < 0)
                ipv4_value_ = 0;
            else if (ipv6_value_ > 255)
                ipv4_value_ = 255;
            else
                ipv4_value_ = (ipv4_value_type)ipv6_value_;
        }
        else
        {
            if (s != sizeof(ipv4_value_))
            {
                std::length_error ex("multicast hops socket option resize");
                throw_exception(ex);
            }
            ipv6_value_ = ipv4_value_;
        }
    }

private:
    ipv4_value_type ipv4_value_;
    ipv6_value_type ipv6_value_;
};

// Helper template for implementing ip_mreq-based options.
template <int IPv4_Level, int IPv4_Name, int IPv6_Level, int IPv6_Name>
class multicast_request
{
public:
    // Default constructor.
    multicast_request()
        : ipv4_value_(), // Zero-initialisation gives the "any" address.
        ipv6_value_() // Zero-initialisation gives the "any" address.
    {
    }

    // Construct with multicast address only.
    explicit multicast_request(const NetLite::ip::address& multicast_address)
        : ipv4_value_(), // Zero-initialisation gives the "any" address.
        ipv6_value_() // Zero-initialisation gives the "any" address.
    {
        if (multicast_address.is_v6())
        {
            using namespace std; // For memcpy.
            NetLite::ip::address_v6 ipv6_address = multicast_address.to_v6();
            NetLite::ip::address_v6::bytes_type bytes = ipv6_address.to_bytes();
            memcpy(ipv6_value_.ipv6mr_multiaddr.s6_addr, bytes.data(), 16);
            ipv6_value_.ipv6mr_interface = ipv6_address.scope_id();
        }
        else
        {
            ipv4_value_.imr_multiaddr.s_addr =
                NetLite::socket_ops::host_to_network_long(
                multicast_address.to_v4().to_ulong());
            ipv4_value_.imr_interface.s_addr =
                NetLite::socket_ops::host_to_network_long(
                NetLite::ip::address_v4::any().to_ulong());
        }
    }

    // Construct with multicast address and IPv4 address specifying an interface.
    explicit multicast_request(
        const NetLite::ip::address_v4& multicast_address,
        const NetLite::ip::address_v4& network_interface
        = NetLite::ip::address_v4::any())
        : ipv6_value_() // Zero-initialisation gives the "any" address.
    {
        ipv4_value_.imr_multiaddr.s_addr =
            NetLite::socket_ops::host_to_network_long(
            multicast_address.to_ulong());
        ipv4_value_.imr_interface.s_addr =
            NetLite::socket_ops::host_to_network_long(
            network_interface.to_ulong());
    }

    // Construct with multicast address and IPv6 network interface index.
    explicit multicast_request(
        const NetLite::ip::address_v6& multicast_address,
        unsigned long network_interface = 0)
        : ipv4_value_() // Zero-initialisation gives the "any" address.
    {
        using namespace std; // For memcpy.
        NetLite::ip::address_v6::bytes_type bytes =
            multicast_address.to_bytes();
        memcpy(ipv6_value_.ipv6mr_multiaddr.s6_addr, bytes.data(), 16);
        if (network_interface)
            ipv6_value_.ipv6mr_interface = network_interface;
        else
            ipv6_value_.ipv6mr_interface = multicast_address.scope_id();
    }

    // Get the level of the socket option.
    template <typename Protocol>
    int level(const Protocol& protocol) const
    {
        if (protocol.family() == PF_INET6)
            return IPv6_Level;
        return IPv4_Level;
    }

    // Get the name of the socket option.
    template <typename Protocol>
    int name(const Protocol& protocol) const
    {
        if (protocol.family() == PF_INET6)
            return IPv6_Name;
        return IPv4_Name;
    }

    // Get the address of the option data.
    template <typename Protocol>
    const void* data(const Protocol& protocol) const
    {
        if (protocol.family() == PF_INET6)
            return &ipv6_value_;
        return &ipv4_value_;
    }

    // Get the size of the option data.
    template <typename Protocol>
    std::size_t size(const Protocol& protocol) const
    {
        if (protocol.family() == PF_INET6)
            return sizeof(ipv6_value_);
        return sizeof(ipv4_value_);
    }

private:
    NetLite::in4_mreq_type ipv4_value_;
    NetLite::in6_mreq_type ipv6_value_;
};

// Helper template for implementing options that specify a network interface.
template <int IPv4_Level, int IPv4_Name, int IPv6_Level, int IPv6_Name>
class network_interface
{
public:
    // Default constructor.
    network_interface()
    {
        ipv4_value_.s_addr =
            NetLite::socket_ops::host_to_network_long(
            NetLite::ip::address_v4::any().to_ulong());
        ipv6_value_ = 0;
    }

    // Construct with IPv4 interface.
    explicit network_interface(const NetLite::ip::address_v4& ipv4_interface)
    {
        ipv4_value_.s_addr =
            NetLite::socket_ops::host_to_network_long(
            ipv4_interface.to_ulong());
        ipv6_value_ = 0;
    }

    // Construct with IPv6 interface.
    explicit network_interface(unsigned int ipv6_interface)
    {
        ipv4_value_.s_addr =
            NetLite::socket_ops::host_to_network_long(
            NetLite::ip::address_v4::any().to_ulong());
        ipv6_value_ = ipv6_interface;
    }

    // Get the level of the socket option.
    template <typename Protocol>
    int level(const Protocol& protocol) const
    {
        if (protocol.family() == PF_INET6)
            return IPv6_Level;
        return IPv4_Level;
    }

    // Get the name of the socket option.
    template <typename Protocol>
    int name(const Protocol& protocol) const
    {
        if (protocol.family() == PF_INET6)
            return IPv6_Name;
        return IPv4_Name;
    }

    // Get the address of the option data.
    template <typename Protocol>
    const void* data(const Protocol& protocol) const
    {
        if (protocol.family() == PF_INET6)
            return &ipv6_value_;
        return &ipv4_value_;
    }

    // Get the size of the option data.
    template <typename Protocol>
    std::size_t size(const Protocol& protocol) const
    {
        if (protocol.family() == PF_INET6)
            return sizeof(ipv6_value_);
        return sizeof(ipv4_value_);
    }

private:
    NetLite::in4_addr_type ipv4_value_;
    unsigned int ipv6_value_;
};

} // namespace socket_option
} // namespace NetLite


#endif // END OF NETLITE_SOCKET_OPTION_HPP
