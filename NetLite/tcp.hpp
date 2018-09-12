
#ifndef NETLITE_TCP_HPP
#define NETLITE_TCP_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "NetLite/basic_socket.hpp"
#include "NetLite/basic_endpoint.hpp"
#include "NetLite/socket_option.hpp"
#include "NetLite/socket_base.hpp"
#include "NetLite/winsock_init.hpp"
namespace NetLite{
/**
 * Encapsulates the flags needed for TCP.
 * The NetLite::ip::tcp class contains flags necessary for TCP sockets.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Safe.
 *
 * @par Concepts:
 * Protocol, InternetProtocol.
 */
class tcp
{
public:
    /// The type of a TCP endpoint.
    typedef basic_endpoint<tcp> endpoint;
    /// The TCP socket type.
    typedef basic_socket<tcp> socket;

    tcp()
        : family_(NET_OS_DEF(AF_INET))
    {

    }

    /// Construct to represent the IPv4 TCP protocol.
    static tcp v4()
    {
        return tcp(NET_OS_DEF(AF_INET));
    }

    /// Construct to represent the IPv6 TCP protocol.
    static tcp v6()
    {
        return tcp(NET_OS_DEF(AF_INET6));
    }

    /// Obtain an identifier for the type of the protocol.
    int type() const
    {
        return NET_OS_DEF(SOCK_STREAM);
    }

    /// Obtain an identifier for the protocol.
    int protocol() const
    {
        return NET_OS_DEF(IPPROTO_TCP);
    }

    /// Obtain an identifier for the protocol family.
    int family() const
    {
        return family_;
    }

    /**
    * Socket option for disabling the Nagle algorithm.
    * Implements the IPPROTO_TCP/TCP_NODELAY socket option.
    *
    * @par Examples
    * Setting the option:
    * @code
    * NetLite::ip::tcp::socket socket(io_context);
    * ...
    * NetLite::ip::tcp::no_delay option(true);
    * socket.set_option(option);
    * @endcode
    *
    * @par
    * Getting the current option value:
    * @code
    * NetLite::ip::tcp::socket socket(io_context);
    * ...
    * NetLite::ip::tcp::no_delay option;
    * socket.get_option(option);
    * bool is_set = option.value();
    * @endcode
    *
    * @par Concepts:
    * Socket_Option, Boolean_Socket_Option.
    */
    typedef NetLite::socket_option::boolean<NET_OS_DEF(IPPROTO_TCP), NET_OS_DEF(TCP_NODELAY) > no_delay;


    /// Compare two protocols for equality.
    friend bool operator==(const tcp& p1, const tcp& p2)
    {
        return p1.family_ == p2.family_;
    }

    /// Compare two protocols for inequality.
    friend bool operator!=(const tcp& p1, const tcp& p2)
    {
        return p1.family_ != p2.family_;
    }

private:
    // Construct with a specific family.
    explicit tcp(int protocol_family)
        : family_(protocol_family)
    {
    }

    int family_;
};
} // namespace NetLite

#endif // END OF NETLITE_TCP_HPP
