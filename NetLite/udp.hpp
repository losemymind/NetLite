
#ifndef NETLITE_UDP_HPP
#define NETLITE_UDP_HPP


#include "NetLite/basic_socket.hpp"
#include "NetLite/basic_endpoint.hpp"
#include "NetLite/socket_base.hpp"

namespace NetLite{

/**
 * Encapsulates the flags needed for UDP.
 * The NetLite::ip::udp class contains flags necessary for UDP sockets.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Safe.
 *
 * @par Concepts:
 * Protocol, InternetProtocol.
 */
class udp
{
public:
    /// The type of a UDP endpoint.
    typedef basic_endpoint<udp> endpoint;
    /// The UDP socket type.
    typedef basic_socket<udp> socket;

    udp()
        : family_(NET_OS_DEF(AF_INET))
    {

    }
    /// Construct to represent the IPv4 UDP protocol.
    static udp v4()
    {
        return udp(NET_OS_DEF(AF_INET));
    }

    /// Construct to represent the IPv6 UDP protocol.
    static udp v6()
    {
        return udp(NET_OS_DEF(AF_INET6));
    }

    /// Obtain an identifier for the type of the protocol.
    int type() const
    {
        return NET_OS_DEF(SOCK_DGRAM);
    }

    /// Obtain an identifier for the protocol.
    int protocol() const
    {
        return NET_OS_DEF(IPPROTO_UDP);
    }

    /// Obtain an identifier for the protocol family.
    int family() const
    {
        return family_;
    }

    /// Compare two protocols for equality.
    friend bool operator==(const udp& p1, const udp& p2)
    {
        return p1.family_ == p2.family_;
    }

    /// Compare two protocols for inequality.
    friend bool operator!=(const udp& p1, const udp& p2)
    {
        return p1.family_ != p2.family_;
    }

private:
    // Construct with a specific family.
    explicit udp(int protocol_family)
        : family_(protocol_family)
    {
    }

    int family_;
};

} // namespace NetLite

#endif // END OF NETLITE_UDP_HPP
