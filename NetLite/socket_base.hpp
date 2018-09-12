
#ifndef NETLITE_SOCKET_BASE_HPP
#define NETLITE_SOCKET_BASE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)


#include "NetLite/socket_option.hpp"


namespace NetLite{

class socket_base
{
public:
    socket_base() { }

    virtual ~socket_base(){ }

    /// Different ways a socket may be shutdown.
    enum shutdown_type
    {
        shutdown_receive = NET_OS_DEF(SHUT_RD),
        shutdown_send    = NET_OS_DEF(SHUT_WR),
        shutdown_both    = NET_OS_DEF(SHUT_RDWR)
    };

    /// Wait types.
    /**
    * For use with basic_socket::wait() and basic_socket::async_wait().
    */
    enum wait_type
    {
        /// Wait for a socket to become ready to read.
        wait_read,

        /// Wait for a socket to become ready to write.
        wait_write,

        /// Wait for a socket to have error conditions pending.
        wait_error
    };

    /**
    * Enumerates BSD socket state parameters.
    */
    enum socket_state
    {
        /// The socket can be for read.
        readable,
        /// The socket can be for write.
        writable,
        /// The socket has error.
        haserror,
    };

    /**
    * Enumerates socket state return values.
    */
    enum class state_return
    {
        Yes,
        No,
        EncounteredError,
    };

    /** Indicates the connection state of the socket */
    enum class connection_state
    {
        NotConnected,
        Connected,
        /** Indicates that the end point refused the connection or couldn't be reached */
        ConnectionError
    };


    /// Bitmask type for flags that can be passed to send and receive operations.
    typedef int message_flags;
    static const int message_peek = NET_OS_DEF(MSG_PEEK);
    static const int message_out_of_band = NET_OS_DEF(MSG_OOB);
    static const int message_do_not_route = NET_OS_DEF(MSG_DONTROUTE);
    static const int message_end_of_record = NET_OS_DEF(MSG_EOR);

    /// The maximum length of the queue of pending incoming connections.
    static const int max_connections = NET_OS_DEF(SOMAXCONN);

public: 
    /// socket options,see NetLite::socket_base


    /**
     * Socket option to permit sending of broadcast messages.
     * Implements the SOL_SOCKET/SO_BROADCAST socket option.
     *
     * @par Examples
     * Setting the option:
     * @code
     * NetLite::udp::socket socket();
     * ...
     * NetLite::socket_base::broadcast option(true);
     * socket.set_option(option);
     * @endcode
     *
     * @par
     * Getting the current option value:
     * @code
     * NetLite::udp::socket socket();
     * ...
     * NetLite::socket_base::broadcast option;
     * socket.get_option(option);
     * bool is_set = option.value();
     * @endcode
     *
     * @par Concepts:
     * Socket_Option, Boolean_Socket_Option.
     */
    typedef NetLite::socket_option::boolean <NET_OS_DEF(SOL_SOCKET), NET_OS_DEF(SO_BROADCAST)>broadcast;


    /**
     * Socket option to enable socket-level debugging.
     * Implements the SOL_SOCKET/SO_DEBUG socket option.
     *
     * @par Examples
     * Setting the option:
     * @code
     * NetLite::ip::tcp::socket socket();
     * ...
     * NetLite::socket_base::debug option(true);
     * socket.set_option(option);
     * @endcode
     *
     * @par
     * Getting the current option value:
     * @code
     * NetLite::ip::tcp::socket socket();
     * ...
     * NetLite::socket_base::debug option;
     * socket.get_option(option);
     * bool is_set = option.value();
     * @endcode
     *
     * @par Concepts:
     * Socket_Option, Boolean_Socket_Option.
     */
    typedef NetLite::socket_option::boolean <NET_OS_DEF(SOL_SOCKET), NET_OS_DEF(SO_DEBUG) > debug;


    /**
     * Socket option to prevent routing, use local interfaces only.
     * Implements the SOL_SOCKET/SO_DONTROUTE socket option.
     *
     * @par Examples
     * Setting the option:
     * @code
     * NetLite::ip::udp::socket socket();
     * ...
     * NetLite::socket_base::do_not_route option(true);
     * socket.set_option(option);
     * @endcode
     *
     * @par
     * Getting the current option value:
     * @code
     * NetLite::ip::udp::socket socket();
     * ...
     * NetLite::socket_base::do_not_route option;
     * socket.get_option(option);
     * bool is_set = option.value();
     * @endcode
     *
     * @par Concepts:
     * Socket_Option, Boolean_Socket_Option.
     */
    typedef NetLite::socket_option::boolean<NET_OS_DEF(SOL_SOCKET), NET_OS_DEF(SO_DONTROUTE)>do_not_route;


    /**
     * Socket option to send keep-alives.
     * Implements the SOL_SOCKET/SO_KEEPALIVE socket option.
     *
     * @par Examples
     * Setting the option:
     * @code
     * NetLite::ip::tcp::socket socket();
     * ...
     * NetLite::socket_base::keep_alive option(true);
     * socket.set_option(option);
     * @endcode
     *
     * @par
     * Getting the current option value:
     * @code
     * NetLite::ip::tcp::socket socket();
     * ...
     * NetLite::socket_base::keep_alive option;
     * socket.get_option(option);
     * bool is_set = option.value();
     * @endcode
     *
     * @par Concepts:
     * Socket_Option, Boolean_Socket_Option.
     */
    typedef NetLite::socket_option::boolean < NET_OS_DEF(SOL_SOCKET), NET_OS_DEF(SO_KEEPALIVE) > keep_alive;


    /**
     * Socket option for the send buffer size of a socket.
     * Implements the SOL_SOCKET/SO_SNDBUF socket option.
     *
     * @par Examples
     * Setting the option:
     * @code
     * NetLite::ip::tcp::socket socket();
     * ...
     * NetLite::socket_base::send_buffer_size option(8192);
     * socket.set_option(option);
     * @endcode
     *
     * @par
     * Getting the current option value:
     * @code
     * NetLite::ip::tcp::socket socket();
     * ...
     * NetLite::socket_base::send_buffer_size option;
     * socket.get_option(option);
     * int size = option.value();
     * @endcode
     *
     * @par Concepts:
     * Socket_Option, Integer_Socket_Option.
     */
    typedef NetLite::socket_option::integer <NET_OS_DEF(SOL_SOCKET), NET_OS_DEF(SO_SNDBUF)>send_buffer_size;

    /**
     * Socket option for the send low watermark.
     * Implements the SOL_SOCKET/SO_SNDLOWAT socket option.
     *
     * @par Examples
     * Setting the option:
     * @code
     * NetLite::ip::tcp::socket socket();
     * ...
     * NetLite::socket_base::send_low_watermark option(1024);
     * socket.set_option(option);
     * @endcode
     *
     * @par
     * Getting the current option value:
     * @code
     * NetLite::ip::tcp::socket socket();
     * ...
     * NetLite::socket_base::send_low_watermark option;
     * socket.get_option(option);
     * int size = option.value();
     * @endcode
     *
     * @par Concepts:
     * Socket_Option, Integer_Socket_Option.
     */
    typedef NetLite::socket_option::integer<NET_OS_DEF(SOL_SOCKET), NET_OS_DEF(SO_SNDLOWAT)>send_low_watermark;

    /**
     * Socket option for the receive buffer size of a socket.
     * Implements the SOL_SOCKET/SO_RCVBUF socket option.
     *
     * @par Examples
     * Setting the option:
     * @code
     * NetLite::ip::tcp::socket socket();
     * ...
     * NetLite::socket_base::receive_buffer_size option(8192);
     * socket.set_option(option);
     * @endcode
     *
     * @par
     * Getting the current option value:
     * @code
     * NetLite::ip::tcp::socket socket();
     * ...
     * NetLite::socket_base::receive_buffer_size option;
     * socket.get_option(option);
     * int size = option.value();
     * @endcode
     *
     * @par Concepts:
     * Socket_Option, Integer_Socket_Option.
     */
    typedef NetLite::socket_option::integer < NET_OS_DEF(SOL_SOCKET), NET_OS_DEF(SO_RCVBUF) > receive_buffer_size;

    /**
     * Socket option for the receive low watermark.
     * Implements the SOL_SOCKET/SO_RCVLOWAT socket option.
     *
     * @par Examples
     * Setting the option:
     * @code
     * NetLite::ip::tcp::socket socket();
     * ...
     * NetLite::socket_base::receive_low_watermark option(1024);
     * socket.set_option(option);
     * @endcode
     *
     * @par
     * Getting the current option value:
     * @code
     * NetLite::ip::tcp::socket socket();
     * ...
     * NetLite::socket_base::receive_low_watermark option;
     * socket.get_option(option);
     * int size = option.value();
     * @endcode
     *
     * @par Concepts:
     * Socket_Option, Integer_Socket_Option.
     */
    typedef NetLite::socket_option::integer<NET_OS_DEF(SOL_SOCKET), NET_OS_DEF(SO_RCVLOWAT)>receive_low_watermark;


    /**
     * Socket option to allow the socket to be bound to an address that is
     * already in use, For accept option.
     * Implements the SOL_SOCKET/SO_REUSEADDR socket option.
     *
     * @par Examples
     * Setting the option:
     * @code
     * NetLite::ip::tcp::socket socket();
     * ...
     * NetLite::socket_base::reuse_address option(true);
     * socket.set_option(option);
     * @endcode
     *
     * @par
     * Getting the current option value:
     * @code
     * NetLite::ip::tcp::socket socket();
     * ...
     * NetLite::socket_base::reuse_address option;
     * socket.get_option(option);
     * bool is_set = option.value();
     * @endcode
     *
     * @par Concepts:
     * Socket_Option, Boolean_Socket_Option.
     */
    typedef NetLite::socket_option::boolean <NET_OS_DEF(SOL_SOCKET), NET_OS_DEF(SO_REUSEADDR) > reuse_address;


    /**
     * Socket option to specify whether the socket lingers on close if unsent
     * data is present.
     *
     * Implements the SOL_SOCKET/SO_LINGER socket option.
     * @par Examples
     * Setting the option:
     * @code
     * NetLite::ip::tcp::socket socket();
     * ...
     * NetLite::socket_base::linger option(true, 30);
     * socket.set_option(option);
     * @endcode
     *
     * @par
     * Getting the current option value:
     * @code
     * NetLite::ip::tcp::socket socket();
     * ...
     * NetLite::socket_base::linger option;
     * socket.get_option(option);
     * bool is_set = option.enabled();
     * unsigned short timeout = option.timeout();
     * @endcode
     *
     * @par Concepts:
     * Socket_Option, Linger_Socket_Option.
     */
    typedef NetLite::socket_option::linger  <NET_OS_DEF(SOL_SOCKET), NET_OS_DEF(SO_LINGER)> linger;

    /**
     * Socket option for determining whether an IPv6 socket supports IPv6
     * communication only.
     * Implements the IPPROTO_IPV6/IP_V6ONLY socket option.
     *
     * @par Examples
     * Setting the option:
     * @code
     * NetLite::ip::tcp::socket socket(io_context);
     * ...
     * NetLite::ip::v6_only option(true);
     * socket.set_option(option);
     * @endcode
     *
     * @par
     * Getting the current option value:
     * @code
     * NetLite::ip::tcp::socket socket(io_context);
     * ...
     * NetLite::ip::v6_only option;
     * socket.get_option(option);
     * bool v6_only = option.value();
     * @endcode
     *
     * @par Concepts:
     * GettableSocketOption, SettableSocketOption.
     */
    typedef NetLite::socket_option::boolean< IPPROTO_IPV6, IPV6_V6ONLY> v6_only;

};

} // namespace NetLite

#endif // END OF NETLITE_SOCKET_BASE_HPP
