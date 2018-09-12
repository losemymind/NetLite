
#ifndef NETLITE_BASIC_SOCKET_HPP
#define NETLITE_BASIC_SOCKET_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <string>
#include <functional>
#include <algorithm>
#include <memory>
#include <vector>
#include <cassert>
#include "NetLite/net_error_code.hpp"
#include "NetLite/socket_types.hpp"
#include "NetLite/socket_base.hpp"
#include "NetLite/socket_ops.hpp"

namespace NetLite{

template<typename Protocol>
class basic_socket : public socket_base
{
public:
    /// The protocol type.
    typedef Protocol protocol_type;

    /// The socket type
    typedef socket_type  native_handle_type;

    /// The endpoint type.
    typedef typename Protocol::endpoint endpoint_type;

    typedef std::shared_ptr<native_handle_type> shared_socket;


    typedef std::function<bool()>                                   handle_method_type;
    typedef std::function<void(const std::error_code)>              async_connect_handler;
    typedef std::function<void(const std::error_code, std::size_t)> async_send_handler;
    typedef std::function<void(const std::error_code, std::size_t)> async_recv_handler;
    typedef std::function<void(const std::error_code, typename Protocol::socket)> async_accept_handler;


public:

    basic_socket()
        : _shared_socket()
        , _state(0)
        , _open(false)

    {
    }

    basic_socket(const protocol_type& protocol, const native_handle_type& native_socket, std::error_code& ec)
    {
        assign(protocol, native_socket, ec);
    }

    /**
     * Move-construct a basic_socket from another.
     * This constructor moves a socket from one object to another.
     *
     * @param other The other basic_socket object from which the move will
     * occur.
     */
    basic_socket(basic_socket&& other)
    {
        move(std::forward<basic_socket<Protocol> >(other));
    }

    /**
     * Copy-construct a basic_socket from another.
     * This constructor copy a socket from one object to another.
     *
     * @param other The other basic_socket object from which the copy will
     * occur.
     */
    basic_socket(const basic_socket& other)
    {
        copy(other);
    }

    /**
     * Move-assign a basic_socket from another.
     * This assignment operator moves a socket from one object to another.
     *
     * @param other The other basic_socket object from which the move will
     * occur.
     */
    basic_socket& operator=(basic_socket&& other)
    {
        move(std::forward<basic_socket<Protocol> >(other));
        return *this;
    }


    /**
     * Copy-assign a basic_socket from another.
     * This assignment operator copy a socket from one object to another.
     *
     * @param other The other basic_socket object from which the copy will
     * occur.
     */
    basic_socket& operator=(const basic_socket& other)
    {
        copy(other);
        return *this;
    }

    /**
     * Virtual destructor
     */
    virtual ~basic_socket()
    {
        if (_shared_socket && _shared_socket.use_count() == 1)
        {
            std::error_code ec;
            shutdown(shutdown_type::shutdown_both, ec);
            if (ec.value() != (int)std::errc::not_connected)
            {
                throw_if(ec);
            }
            this->close();
        }
    }

    virtual void poll()
    {

    }

    /**
     * Open the socket using the specified protocol.
     * This function opens the socket so that it will use the specified protocol.
     *
     * @param protocol An object specifying protocol parameters to be used.
     *
     * @throws std::system_error Thrown on failure.
     *
     * @par Example
     * @code
     * ip::tcp::socket socket();
     * socket.open(ip::tcp::v4());
     * @endcode
     */
    virtual void open(const protocol_type& protocol = protocol_type())
    {
        std::error_code ec;
        this->open(protocol, ec);
        throw_if(ec, "open");
    }

    /**
     * Open the socket using the specified protocol.
     * This function opens the socket so that it will use the specified protocol.
     *
     * @param protocol An object specifying which protocol is to be used.
     *
     * @param ec Set to indicate what error occurred, if any.
     *
     * @par Example
     * @code
     * ip::tcp::socket socket();
     * std::error_code ec;
     * socket.open(ip::tcp::v4(), ec);
     * if (ec)
     * {
     *   // An error occurred.
     * }
     * @endcode
     */
    virtual std::error_code open(const protocol_type& protocol, std::error_code& ec)
    {
        native_handle_type native_socket = socket_ops::socket(protocol.family(), protocol.type(), protocol.protocol(), ec);
        holdsSocket(native_socket);
        switch (protocol.type())
        {
        case SOCK_STREAM: _state = socket_ops::stream_oriented; break;
        case SOCK_DGRAM: _state  = socket_ops::datagram_oriented; break;
        default: _state = 0; break;
        }
        if (!ec)
            _open = true;
        return ec;
    }

    /**
     * Assign an existing native socket to the socket.
     * This function opens the socket to hold an existing native socket.
     *
     * @param protocol An object specifying which protocol is to be used.
     *
     * @param native_socket A native socket.
     *
     * @throws std::system_error Thrown on failure.
     */
    void assign(const protocol_type& protocol, const native_handle_type& native_socket)
    {
        std::error_code ec;
        this->assign(protocol, native_socket, ec);
        throw_if(ec, "assign");
    }

    /**
     * Assign an existing native socket to the socket.
     * This function opens the socket to hold an existing native socket.
     *
     * @param protocol An object specifying which protocol is to be used.
     *
     * @param native_socket A native socket.
     *
     * @param ec Set to indicate what error occurred, if any.
     */
    std::error_code assign(const protocol_type& protocol, const native_handle_type& native_socket, std::error_code& ec)
    {
        holdsSocket(native_socket);
        switch (protocol.type())
        {
        case SOCK_STREAM:
            _state = socket_ops::stream_oriented;
            break;
        case SOCK_DGRAM:
            _state = socket_ops::datagram_oriented;
            break;
        default:
            _state = 0;
            break;
        }
        _state |= socket_ops::possible_dup;
        _protocol = protocol;
        if (!ec)
            _open = true;
        return ec;
    }

    /// Determine whether the socket is open.
    bool is_open() const
    {
        return _open;
    }

    /**
     * Connect the socket to the specified endpoint.
     * This function is used to connect a socket to the specified remote endpoint.
     * The function call will block until the connection is successfully made or
     * an error occurs.
     *
     * The socket is automatically opened if it is not already open. If the
     * connect fails, and the socket was automatically opened, the socket is
     * not returned to the closed state.
     *
     * @param peer_endpoint The remote endpoint to which the socket will be
     * connected.
     *
     * @throws std::system_error Thrown on failure.
     *
     * @par Example
     * @code
     * ip::tcp::socket socket();
     * ip::tcp::endpoint endpoint(ip::address::from_string("1.2.3.4"), 12345);
     * socket.connect(endpoint);
     * @endcode
     */
    void connect(const endpoint_type& peer_endpoint)
    {
        std::error_code ec;
        this->connect(peer_endpoint, ec);
        throw_if(ec, "connect");
    }

    /**
     * Connect the socket to the specified endpoint.
     * This function is used to connect a socket to the specified remote endpoint.
     * The function call will block until the connection is successfully made or
     * an error occurs.
     *
     * The socket is automatically opened if it is not already open. If the
     * connect fails, and the socket was automatically opened, the socket is
     * not returned to the closed state.
     *
     * @param peer_endpoint The remote endpoint to which the socket will be
     * connected.
     *
     * @param ec Set to indicate what error occurred, if any.
     *
     * @par Example
     * @code
     * ip::tcp::socket socket();
     * ip::tcp::endpoint endpoint(ip::address::from_string("1.2.3.4"), 12345);
     * std::error_code ec;
     * socket.connect(endpoint, ec);
     * if (ec)
     * {
     *   // An error occurred.
     * }
     * @endcode
     */
    std::error_code connect(const endpoint_type& peer_endpoint, std::error_code& ec)
    {
        if (!this->is_open())
        {
            this->open(peer_endpoint.protocol(), ec);
            throw_if(ec, "connect");
        }
        socket_ops::sync_connect(native_handle(), peer_endpoint.data(), peer_endpoint.size(), ec);
        return ec;
    }

    /**
     * Bind the socket to the given local endpoint.
     * This function binds the socket to the specified endpoint on the local
     * machine.
     *
     * @param endpoint An endpoint on the local machine to which the socket will
     * be bound.
     *
     * @throws std::system_error Thrown on failure.
     *
     * @par Example
     * @code
     * ip::tcp::socket socket();
     * socket.open(ip::tcp::v4());
     * socket.bind(ip::tcp::endpoint(ip::tcp::v4(), 12345));
     * @endcode
     */
    void bind(const endpoint_type& endpoint)
    {
        std::error_code ec;
        this->bind(endpoint,ec);
        throw_if(ec, "bind");
    }

    /**
     * Bind the socket to the given local endpoint.
     * This function binds the socket to the specified endpoint on the local
     * machine.
     *
     * @param endpoint An endpoint on the local machine to which the socket will
     * be bound.
     *
     * @param ec Set to indicate what error occurred, if any.
     *
     * @par Example
     * @code
     * ip::tcp::socket socket();
     * socket.open(ip::tcp::v4());
     * std::error_code ec;
     * socket.bind(ip::tcp::endpoint(ip::tcp::v4(), 12345), ec);
     * if (ec)
     * {
     *   // An error occurred.
     * }
     * @endcode
     */
    std::error_code bind(const endpoint_type& endpoint, std::error_code& ec)
    {
        socket_ops::bind(native_handle(), endpoint.data(), endpoint.size(), ec);
        return ec;
    }


    /**
     * Send some data on the socket.
     * This function is used to send data on the socket. The function
     * call will block until one or more bytes of the data has been sent
     * successfully, or an until error occurs.
     *
     * @param buffers One or more data buffers to be sent on the socket.
     *
     * @param flags Flags specifying how the send call is to be made.
     *
     * @returns The number of bytes sent.
     *
     * @throws std::system_error Thrown on failure.
     *
     * @note The send operation may not transmit all of the data to the peer.
     * Consider using the @ref write function if you need to ensure that all data
     * is written before the blocking operation completes.
     *
     * @par Example
     * To send a single data buffer use the @ref buffer function as follows:
     * @code
     * socket.send(mutable_buffer(data, size), 0);
     * @endcode
     * See the @ref buffer documentation for information on sending multiple
     * data in one go, and how to use it with arrays ,std::string or
     * std::vector.
     */
    std::size_t send(const std::vector<char>& buffers, socket_base::message_flags flags = 0)
    {
        std::error_code ec;
        std::size_t len = this->send(buffers, flags, ec);
        throw_if(ec, "send");
        return len;
    }

    /**
     * Send some data on the socket.
     * This function is used to send data on the stream socket. The function
     * call will block until one or more bytes of the data has been sent
     * successfully, or an until error occurs.
     *
     * @param buffers One or more data buffers to be sent on the socket.
     *
     * @param flags Flags specifying how the send call is to be made.
     *
     * @param ec Set to indicate what error occurred, if any.
     *
     * @returns The number of bytes sent. Returns 0 if an error occurred.
     *
     * @note The send operation may not transmit all of the data to the peer.
     * Consider using the @ref write function if you need to ensure that all data
     * is written before the blocking operation completes.
     */
    std::size_t send(const std::vector<char>& buffers, socket_base::message_flags flags, std::error_code& ec)
    {
        if ((_state & socket_ops::stream_oriented))
        {
            socket_ops::buf sendBuf;
            socket_ops::init_buf(sendBuf, buffers.data(), buffers.size());
            return socket_ops::send(native_handle(),&sendBuf ,buffers.size(), flags, ec);
        }
        else
        {
            std::error_code ec = make_error_code(std::errc::address_family_not_supported);
            throw_if(ec, "send");
        }
        return 0;
    }

    /**
     * Receive some data on the socket.
     * This function is used to receive data on the stream socket. The function
     * call will block until one or more bytes of data has been received
     * successfully, or until an error occurs.
     *
     * @param buffers One or more buffers into which the data will be received.
     *
     * @param flags Flags specifying how the receive call is to be made,
     *              Default is 0.
     *
     * @returns The number of bytes received.
     *
     * @throws std::system_error Thrown on failure. An error code of
     * std::errc::eof indicates that the connection was closed by the
     * peer.
     *
     * @note The receive operation may not receive all of the requested number of
     * bytes. Consider using the @ref read function if you need to ensure that the
     * requested amount of data is read before the blocking operation completes.
     *
     * @par Example
     * To receive into a single data buffer use the @ref buffer function as
     * follows:
     * @code
     * socket.receive(mutable_buffer(data, size), 0);
     * @endcode
     * See the @ref buffer documentation for information on receiving into
     * multiple buffers in one go, and how to use it with arrays, boost::array or
     * std::vector.
     */
    std::size_t receive(std::vector<char>& buffers, socket_base::message_flags flags = 0)
    {
        std::error_code ec;
        std::size_t result = this->receive(buffers, ec, flags);
        throw_if(ec, "receive");
        return result;
    }

    /**
     * Receive some data on a connected socket.
     * This function is used to receive data on the stream socket. The function
     * call will block until one or more bytes of data has been received
     * successfully, or until an error occurs.
     *
     * @param buffers One or more buffers into which the data will be received.
     *
     * @param ec Set to indicate what error occurred, if any.
     *
     * @param flags Flags specifying how the receive call is to be made.
     *
     * @returns The number of bytes received. Returns 0 if an error occurred.
     *
     * @note The receive operation may not receive all of the requested number of
     * bytes. Consider using the @ref read function if you need to ensure that the
     * requested amount of data is read before the blocking operation completes.
     */
    std::size_t receive(std::vector<char>& buffers, std::error_code& ec, socket_base::message_flags flags = 0)
    {
        if ((_state & socket_ops::stream_oriented))
        {
            socket_ops::buf recvBuf;
            socket_ops::init_buf(recvBuf, buffers.data(), buffers.size());
            return socket_ops::recv(native_handle(), &recvBuf, buffers.size(), flags, ec);
        }
        else
        {
            std::error_code ec = make_error_code(std::errc::address_family_not_supported);
            throw_if(ec, "receive address family not suported.");
        }
        return 0;
    }

    /**
     * Send a datagram to the specified endpoint.
     * This function is used to send a datagram to the specified remote endpoint.
     * The function call will block until the data has been sent successfully or
     * an error occurs.
     *
     * @param buffers One or more data buffers to be sent to the remote endpoint.
     *
     * @param destination The remote endpoint to which the data will be sent.
     *
     * @param flags Flags specifying how the send call is to be made.
     *
     * @returns The number of bytes sent.
     *
     * @throws std::system_error Thrown on failure.
     */
    std::size_t send_to(std::vector<char>& buffers
        , const endpoint_type& destination
        , socket_base::message_flags flags = 0)
    {
        std::error_code ec;
        std::size_t result = this->send_to(buffers, destination, flags, ec);
        throw_if(ec, "send_to");
        return result;
    }

    /**
     * Send a datagram to the specified endpoint.
     * This function is used to send a datagram to the specified remote endpoint.
     * The function call will block until the data has been sent successfully or
     * an error occurs.
     *
     * @param buffers One or more data buffers to be sent to the remote endpoint.
     *
     * @param destination The remote endpoint to which the data will be sent.
     *
     * @param flags Flags specifying how the send call is to be made.
     *
     * @param ec Set to indicate what error occurred, if any.
     *
     * @returns The number of bytes sent.
     */
    std::size_t send_to(std::vector<char>& buffers
        , const endpoint_type& destination
        , socket_base::message_flags flags
        , std::error_code& ec)
    {

        if ((_state & socket_ops::datagram_oriented))
        {
            socket_ops::buf sendToBuf;
            socket_ops::init_buf(sendToBuf, buffers.data(), buffers.size());
            return socket_ops::sendto(native_handle()
                , &sendToBuf
                , buffers.size()
                , flags
                , destination.data()
                , destination.size()
                , ec);
        }
        else
        {
            std::error_code ec = make_error_code(std::errc::address_family_not_supported);
            throw_if(ec, "send_to");
        }
        return 0;
    }

    /**
     * Receive a datagram with the endpoint of the sender.
     * This function is used to receive a datagram. The function call will block
     * until data has been received successfully or an error occurs.
     *
     * @param buffers One or more buffers into which the data will be received.
     *
     * @param sender_endpoint An endpoint object that receives the endpoint of
     * the remote sender of the datagram.
     *
     * @param flags Flags specifying how the receive call is to be made.
     *              Default is 0.
     * @returns The number of bytes received.
     */
    std::size_t receive_from(std::vector<char>& buffers
        , endpoint_type& sender_endpoint
        , socket_base::message_flags flags = 0)
    {
        std::error_code ec;
        std::size_t result = this->receive_from(buffers, sender_endpoint, flags, ec);
        throw_if(ec, "receive_from");
        return result;
    }

    /**
     * Receive a datagram with the endpoint of the sender.
     * This function is used to receive a datagram. The function call will block
     * until data has been received successfully or an error occurs.
     *
     * @param buffers One or more buffers into which the data will be received.
     *
     * @param sender_endpoint An endpoint object that receives the endpoint of
     * the remote sender of the datagram.
     *
     * @param flags Flags specifying how the receive call is to be made.
     *              Default is 0.
     *
     * @param ec Set to indicate what error occurred, if any.
     *
     * @returns The number of bytes received.
     */
    std::size_t receive_from(std::vector<char>& buffers
        , endpoint_type& sender_endpoint
        , socket_base::message_flags flags
        , std::error_code& ec)
    {
        std::size_t addr_len = sender_endpoint.capacity();
        socket_ops::buf recvFromBuf;
        socket_ops::init_buf(recvFromBuf, buffers.data(), buffers.size());
        std::size_t bytes_recvd = socket_ops::sync_recvfrom(native_handle()
            , _state
            , &recvFromBuf
            , buffers.size()
            , flags
            , sender_endpoint.data()
            , &addr_len
            , ec);

        if (!ec)
            sender_endpoint.resize(addr_len);

        return bytes_recvd;
    }

    /**
     * This function puts the socket acceptor into the state where it may accept
     * new connections.
     *
     * @param backlog The maximum length of the queue of pending connections.
     *
     * @throws std::system_error Thrown on failure.
     */
    void listen(int backlog = socket_base::max_connections)
    {
        std::error_code ec;
        this->listen(backlog, ec);
        throw_if(ec, "listen");
    }


    /**
     * This function puts the socket acceptor into the state where it may accept
     * new connections.
     *
     * @param backlog The maximum length of the queue of pending connections.
     *
     * @param ec Set to indicate what error occurred, if any.
     *
     * @par Example
     * @code
     * std::error_code ec;
     * socket.listen(NetLite::socket_base::max_connections, ec);
     * if (ec)
     * {
     *   // An error occurred.
     * }
     * @endcode
     */
    std::error_code listen(int backlog, std::error_code& ec)
    {
        socket_ops::listen(native_handle(), backlog, ec);
        return ec;
    }

    /**
     * Accept a new connection.
     * This function is used to accept a new connection from a peer. The function
     * call will block until a new connection has been accepted successfully or
     * an error occurs.
     *
     * This overload requires that the Protocol template parameter satisfy the
     * AcceptableProtocol type requirements.
     *
     * @param peer_endpoint An endpoint object into which the endpoint of the
     * remote peer will be written.
     *
     * @returns A socket object representing the newly accepted connection.
     *
     * @throws std::system_error Thrown on failure.
     *
     * @par Example
     * @code
     * ...
     * ip::tcp::endpoint endpoint;
     * ip::tcp::socket socket.accept(endpoint);
     * @endcode
     */
    typename Protocol::socket accept(endpoint_type& peer_endpoint)
    {
        std::error_code ec;
        typename Protocol::socket new_socket = this->accept(peer_endpoint, ec);
        throw_if(ec, "accept");
        return new_socket;
    }

    /**
     * Accept a new connection.
     * This function is used to accept a new connection from a peer. The function
     * call will block until a new connection has been accepted successfully or
     * an error occurs.
     *
     * This overload requires that the Protocol template parameter satisfy the
     * AcceptableProtocol type requirements.
     *
     * @param peer_endpoint An endpoint object into which the endpoint of the
     * remote peer will be written.
     *
     * @param ec Set to indicate what error occurred, if any.
     *
     * @returns On success, a socket object representing the newly accepted
     * connection. On error, a socket object where is_open() is false.
     *
     * @par Example
     * @code
     * ...
     * ip::tcp::endpoint endpoint;
     * ip::tcp::socket socket.accept(endpoint, ec);
     * if (ec)
     * {
     *   // An error occurred.
     * }
     * @endcode
     */
    typename Protocol::socket accept(endpoint_type& peer_endpoint, std::error_code& ec)
    {
        size_t addrLen = peer_endpoint.size();
        native_handle_type native_socket = socket_ops::sync_accept(native_handle()
            , _state
            , peer_endpoint.data()
            , &addrLen
            , ec);
        if (native_socket != invalid_socket)
        {
            peer_endpoint.resize(addrLen);
        }
        typename Protocol::socket new_socket(this->_protocol, native_socket, ec);
        return new_socket;
    }

    //////////////////////////////////////////////////////////////////////////
    /// asynchronous operation functions

    /**
     * Gets the non-blocking mode of the socket.
     * @returns @c true if the socket's synchronous operations will fail with
     * std::errcwould_block if they are unable to perform the requested
     * operation immediately. If @c false, synchronous operations will block
     * until complete.
     *
     * @note The non-blocking mode has no effect on the behaviour of asynchronous
     * operations. Asynchronous operations will never fail with the error
     * std::errcwould_block.
     */
    bool is_non_blocking() const
    {
        return ((_state & socket_ops::non_blocking) != 0);
    }

    /**
     * Sets the non-blocking mode of the socket.
     * @param mode If @c true, the socket's synchronous operations will fail with
     * std::errcwould_block if they are unable to perform the requested
     * operation immediately. If @c false, synchronous operations will block
     * until complete.
     *
     * @throws std::system_error Thrown on failure.
     *
     * @note The non-blocking mode has no effect on the behaviour of asynchronous
     * operations. Asynchronous operations will never fail with the error
     * std::errcwould_block.
     */
    void non_blocking(bool mode)
    {
        std::error_code ec;
        non_blocking(mode, ec);
        throw_if(ec, "non_blocking");
    }

    /**
     * Sets the non-blocking mode of the socket.
     * @param mode If @c true, the socket's synchronous operations will fail with
     * std::errcwould_block if they are unable to perform the requested
     * operation immediately. If @c false, synchronous operations will block
     * until complete.
     *
     * @param ec Set to indicate what error occurred, if any.
     *
     * @note The non-blocking mode has no effect on the behaviour of asynchronous
     * operations. Asynchronous operations will never fail with the error
     * std::errcwould_block.
     */
    std::error_code non_blocking( bool mode, std::error_code& ec)
    {
        socket_ops::set_user_non_blocking(native_handle(), _state, mode, ec);
        return ec;
    }


    /**
     * Start an asynchronous connect.
     * This function is used to asynchronously connect a socket to the specified
     * remote endpoint. The function call always returns immediately.
     *
     * The socket is automatically opened if it is not already open. If the
     * connect fails, and the socket was automatically opened, the socket is
     * not returned to the closed state.
     *
     * @param peer_endpoint The remote endpoint to which the socket will be
     * connected. Copies will be made of the endpoint object as required.
     *
     * @param handler The handler to be called when the connection operation
     * completes. Copies will be made of the handler as required. The function
     * signature of the handler must be:
     * @code void handler(
     *   const std::error_code& error // Result of operation
     * ); @endcode
     * @par Example
     * @code
     * void connect_handler(const std::error_code& error)
     * {
     *   if (!error)
     *   {
     *     // Connect succeeded.
     *   }
     * }
     *
     * ...
     *
     * ip::tcp::socket socket();
     * ip::tcp::endpoint endpoint(ip::address::from_string("1.2.3.4"), 12345);
     * socket.async_connect(endpoint, connect_handler);
     * @endcode
     */
    std::error_code async_connect(const endpoint_type& peer_endpoint, async_connect_handler& handler)
    {
        std::error_code ec;
        // Not Implementation.
        assert(false);
        return ec;
    }

    /**
     * Start an asynchronous send.
     * This function is used to asynchronously send data on the stream socket.
     * The function call always returns immediately.
     *
     * @param buffers One or more data buffers to be sent on the socket. Although
     * the buffers object may be copied as necessary, ownership of the underlying
     * memory blocks is retained by the caller, which must guarantee that they
     * remain valid until the handler is called.
     *
     * @param handler The handler to be called when the send operation completes.
     * Copies will be made of the handler as required. The function signature of
     * the handler must be:
     * @code void handler(
     *   const std::error_code& error,     // Result of operation.
     *   std::size_t sendBytes             // Number of bytes sent.
     * ); @endcode
     *
     * @note The send operation may not transmit all of the data to the peer.
     * Consider using the @ref async_write function if you need to ensure that all
     * data is written before the asynchronous operation completes.
     *
     * @par Example
     * To send a single data buffer use the @ref buffer function as follows:
     * @code
     * socket.async_send(mutable_buffer(data, size), handler);
     * @endcode
     * See the @ref buffer documentation for information on sending multiple
     * buffers in one go, and how to use it with arrays, std::array or
     * std::vector.
     */
    void async_send(std::vector<char>& buffers, async_send_handler& handler)
    {
        this->async_send(buffers, 0, handler);
    }

    /**
     * Start an asynchronous send.
     * This function is used to asynchronously send data on the stream socket.
     * The function call always returns immediately.
     *
     * @param buffers One or more data buffers to be sent on the socket. Although
     * the buffers object may be copied as necessary, ownership of the underlying
     * memory blocks is retained by the caller, which must guarantee that they
     * remain valid until the handler is called.
     *
     * @param flags Flags specifying how the send call is to be made.
     *
     * @param handler The handler to be called when the send operation completes.
     * Copies will be made of the handler as required. The function signature of
     * the handler must be:
     * @code void handler(
     *   const std::error_code& error, // Result of operation.
     *   std::size_t sendBytes           // Number of bytes sent.
     * ); @endcode
     *
     * @note The send operation may not transmit all of the data to the peer.
     * Consider using the @ref async_write function if you need to ensure that all
     * data is written before the asynchronous operation completes.
     *
     * @par Example
     * To send a single data buffer use the @ref buffer function as follows:
     * @code
     * socket.async_send(mutable_buffer(data, size), 0, handler);
     * @endcode
     * See the @ref buffer documentation for information on sending multiple
     * buffers in one go, and how to use it with arrays, std::array or
     * std::vector.
     */
    void async_send(std::vector<char>& buffers, socket_base::message_flags flags, async_send_handler& handler)
    {
        if ((_state & socket_ops::stream_oriented))
        {
            // Not Implementation.
            assert(false);
        }
        else
        {
            std::error_code ec = make_error_code(std::errc::address_family_not_supported);
            throw_if(ec, "async_send");
        }
    }

    /**
     * Start an asynchronous receive.
     * This function is used to asynchronously receive data from the stream
     * socket. The function call always returns immediately.
     *
     * @param buffers One or more buffers into which the data will be received.
     * Although the buffers object may be copied as necessary, ownership of the
     * underlying memory blocks is retained by the caller, which must guarantee
     * that they remain valid until the handler is called.
     *
     * @param handler The handler to be called when the receive operation
     * completes. Copies will be made of the handler as required. The function
     * signature of the handler must be:
     * @code void handler(
     *   const std::error_code& error,  // Result of operation.
     *   std::size_t bytes_transferred  // Number of bytes received.
     * ); @endcode
     *
     * @note The receive operation may not receive all of the requested number of
     * bytes. Consider using the @ref async_read function if you need to ensure
     * that the requested amount of data is received before the asynchronous
     * operation completes.
     *
     * @par Example
     * To receive into a single data buffer use the @ref buffer function as
     * follows:
     * @code
     * socket.async_receive(mutable_buffer(data, size), handler);
     * @endcode
     * See the @ref buffer documentation for information on receiving into
     * multiple buffers in one go, and how to use it with arrays, boost::array or
     * std::vector.
     */
    void async_receive(std::vector<char>& buffers, async_recv_handler& handler)
    {

        this->async_receive(buffers, 0, handler);
    }

    /**
     * Start an asynchronous receive.
     * This function is used to asynchronously receive data from the stream
     * socket. The function call always returns immediately.
     *
     * @param buffers One or more buffers into which the data will be received.
     * Although the buffers object may be copied as necessary, ownership of the
     * underlying memory blocks is retained by the caller, which must guarantee
     * that they remain valid until the handler is called.
     *
     * @param flags Flags specifying how the receive call is to be made.
     *
     * @param handler The handler to be called when the receive operation
     * completes. Copies will be made of the handler as required. The function
     * signature of the handler must be:
     * @code void handler(
     *   const std::error_code& error,     // Result of operation.
     *   std::size_t bytes_transferred     // Number of bytes received.
     * ); @endcode
     *
     * @note The receive operation may not receive all of the requested number of
     * bytes. Consider using the @ref async_read function if you need to ensure
     * that the requested amount of data is received before the asynchronous
     * operation completes.
     *
     * @par Example
     * To receive into a single data buffer use the @ref buffer function as
     * follows:
     * @code
     * socket.async_receive(mutable_buffer(data, size), 0, handler);
     * @endcode
     * See the @ref buffer documentation for information on receiving into
     * multiple buffers in one go, and how to use it with arrays, boost::array or
     * std::vector.
     */
    void async_receive(std::vector<char>& buffers, socket_base::message_flags flags, async_recv_handler& handler)
    {
        if ((_state & socket_ops::stream_oriented))
        {
            // Not Implementation.
            assert(false);
        }
        else
        {
            std::error_code ec = make_error_code(std::errc::address_family_not_supported);
            throw_if(ec, "saync_send");
        }
    }

    /**
     * Start an asynchronous send.
     * This function is used to asynchronously send a datagram to the specified
     * remote endpoint. The function call always returns immediately.
     *
     * @param buffers One or more data buffers to be sent to the remote endpoint.
     * Although the buffers object may be copied as necessary, ownership of the
     * underlying memory blocks is retained by the caller, which must guarantee
     * that they remain valid until the handler is called.
     *
     * @param destination The remote endpoint to which the data will be sent.
     * Copies will be made of the endpoint as required.
     *
     * @param handler The handler to be called when the send operation completes.
     * Copies will be made of the handler as required. The function signature of
     * the handler must be:
     * @code void handler(
     *   const std::error_code& error, // Result of operation.
     *   std::size_t bytes_transferred  // Number of bytes sent.
     * );
     * @endcode
     *
     * @par Example
     * To send a single data buffer use the @ref buffer function as follows:
     * @code
     * ip::udp::endpoint destination(
     *     ip::address::from_string("1.2.3.4"), 12345);
     * socket.async_send_to(
     *     mutable_buffer(data, size), destination, handler);
     * @endcode
     * See the @ref buffer documentation for information on sending multiple
     * buffers in one go, and how to use it with arrays, std::array or
     * std::vector.
     */
    void async_send_to(std::vector<char>& buffers
        , const endpoint_type& destination
        , async_send_handler& handler)
    {
        this->async_send_to(buffers, destination, 0, handler);
    }


    /**
     * Start an asynchronous send.
     * This function is used to asynchronously send a datagram to the specified
     * remote endpoint. The function call always returns immediately.
     *
     * @param buffers One or more data buffers to be sent to the remote endpoint.
     * Although the buffers object may be copied as necessary, ownership of the
     * underlying memory blocks is retained by the caller, which must guarantee
     * that they remain valid until the handler is called.
     *
     * @param flags Flags specifying how the send call is to be made.
     *
     * @param destination The remote endpoint to which the data will be sent.
     * Copies will be made of the endpoint as required.
     *
     * @param handler The handler to be called when the send operation completes.
     * Copies will be made of the handler as required. The function signature of
     * the handler must be:
     * @code void handler(
     *   const std::error_code& error,  // Result of operation.
     *   std::size_t bytes_transferred  // Number of bytes sent.
     * );
     * @endcode
     */
    void async_send_to(std::vector<char>& buffers
        , const endpoint_type& destination
        , socket_base::message_flags flags
        , async_send_handler& handler)
    {
        if ((_state & socket_ops::datagram_oriented))
        {
            // Not Implementation.
            assert(false);
        }
        else
        {
            std::error_code ec = make_error_code(std::errc::address_family_not_supported);
            throw_if(ec, "async_send_to");
        }
    }

    /**
     * Start an asynchronous receive.
     * This function is used to asynchronously receive a datagram. The function
     * call always returns immediately.
     *
     * @param buffers One or more buffers into which the data will be received.
     * Although the buffers object may be copied as necessary, ownership of the
     * underlying memory blocks is retained by the caller, which must guarantee
     * that they remain valid until the handler is called.
     *
     * @param sender_endpoint An endpoint object that receives the endpoint of
     * the remote sender of the datagram. Ownership of the sender_endpoint object
     * is retained by the caller, which must guarantee that it is valid until the
     * handler is called.
     *
     * @param handler The handler to be called when the receive operation
     * completes. Copies will be made of the handler as required. The function
     * signature of the handler must be:
     * @code void handler(
     *   const std::error_code& error, // Result of operation.
     *   std::size_t bytes_transferred           // Number of bytes received.
     * ); @endcode
     *
     * @par Example
     * To receive into a single data buffer use the @ref buffer function as
     * follows:
     * @code socket.async_receive_from(
     *     mutable_buffer(data, size), sender_endpoint, handler);
     * @endcode
     * See the @ref buffer documentation for information on receiving into
     * multiple buffers in one go, and how to use it with arrays, std::array or
     * std::vector.
     */
    void async_receive_from(std::vector<char>& buffers
        , endpoint_type& sender_endpoint
        , async_recv_handler& handler)
    {
        this->async_receive_from(buffers, sender_endpoint, 0, handler);
    }

    /**
     * Start an asynchronous receive.
     * This function is used to asynchronously receive a datagram. The function
     * call always returns immediately.
     *
     * @param buffers One or more buffers into which the data will be received.
     * Although the buffers object may be copied as necessary, ownership of the
     * underlying memory blocks is retained by the caller, which must guarantee
     * that they remain valid until the handler is called.
     *
     * @param sender_endpoint An endpoint object that receives the endpoint of
     * the remote sender of the datagram. Ownership of the sender_endpoint object
     * is retained by the caller, which must guarantee that it is valid until the
     * handler is called.
     *
     * @param flags Flags specifying how the receive call is to be made.
     *
     * @param handler The handler to be called when the receive operation
     * completes. Copies will be made of the handler as required. The function
     * signature of the handler must be:
     * @code void handler(
     *   const std::error_code& error, // Result of operation.
     *   std::size_t bytes_transferred // Number of bytes received.
     * ); @endcode
     */
    void async_receive_from(std::vector<char>& buffers
        , endpoint_type& sender_endpoint
        , socket_base::message_flags flags
        , async_recv_handler& handler)
    {
        if ((_state & socket_ops::datagram_oriented))
        {
            // Not Implementation.
            assert(false);
        }
        else
        {
            std::error_code ec = make_error_code(std::errc::address_family_not_supported);
            throw_if(ec, "async_receive_from");
        }

    }

    /**
     * Start an asynchronous accept.
     * This function is used to asynchronously accept a new connection. The
     * function call always returns immediately.
     *
     * This overload requires that the Protocol template parameter satisfy the
     * AcceptableProtocol type requirements.
     *
     * @param peer_endpoint An endpoint object into which the endpoint of the
     * remote peer will be written. Ownership of the peer_endpoint object is
     * retained by the caller, which must guarantee that it is valid until the
     * handler is called.
     *
     * @param handler The handler to be called when the accept operation
     * completes. Copies will be made of the handler as required. The function
     * signature of the handler must be:
     * @code void handler(
     *   const std::error_code& error, // Result of operation.
     *   typename Protocol::socket peer // On success, the newly accepted socket.
     * ); @endcode
     *
     * @par Example
     * @code
     * void accept_handler(const std::error_code& error,NetLite::ip::tcp::socket peer)
     * {
     *   if (!error)
     *   {
     *     // Accept succeeded.
     *   }
     * }
     *
     * ...
     * NetLite::ip::tcp::endpoint endpoint;
     * ip::tcp::socket socket.accept(endpoint, accept_handler);
     * @endcode
     */
    void async_accept(endpoint_type& peer_endpoint, async_accept_handler& handler)
    {
        if ((_state & socket_ops::datagram_oriented))
        {
            // Not Implementation.
            assert(false);
        }
        else
        {
            std::error_code ec = make_error_code(std::errc::address_family_not_supported);
            throw_if(ec, "async_receive_from");
        }
    }

    /**
     * Asynchronously wait for the socket to become ready to read, ready to
     * write, or to have pending error conditions.
     *
     * This function is used to perform an asynchronous wait for a socket to enter
     * a ready to read, write or error condition state.
     *
     * @param w Specifies the desired socket state.
     *
     * @param handler The handler to be called when the wait operation completes.
     * Copies will be made of the handler as required. The function signature of
     * the handler must be:
     * @code void handler(
     *   const std::error_code& error // Result of operation
     * ); @endcode
     *
     * @par Example
     * @code
     * void wait_handler(const std::error_code& error)
     * {
     *   if (!error)
     *   {
     *     // Wait succeeded.
     *   }
     * }
     *
     * ...
     *
     * ip::tcp::socket socket();
     * ...
     * socket.async_wait(ip::tcp::socket::wait_read, wait_handler);
     * @endcode
     */
    void async_wait(wait_type w, std::function<void(std::error_code)>& handler)
    {
        // Not Implementation.
        assert(false);
    }

    /// asynchronous operation functions
    //////////////////////////////////////////////////////////////////////////


    /**
     * Determine the number of bytes available for reading.
     * This function is used to determine the number of bytes that may be read
     * without blocking.
     *
     * @return The number of bytes that may be read without blocking, or 0 if an
     * error occurs.
     *
     * @throws std::system_error Thrown on failure.
     */
    std::size_t available() const
    {
        std::error_code ec;
        std::size_t result = this->available(ec);
        throw_if(ec, "available");
        return result;
    }

    /**
     * Determine the number of bytes available for reading.
     * This function is used to determine the number of bytes that may be read
     * without blocking.
     *
     * @param ec Set to indicate what error occurred, if any.
     *
     * @return The number of bytes that may be read without blocking, or 0 if an
     * error occurs.
     */
    std::size_t available(std::error_code& ec) const
    {
        return socket_ops::available(native_handle(), ec);
    }

    /**
     * Wait for the socket to become ready to read, ready to write, or to have
     * pending error conditions.
     * This function is used to perform a blocking wait for a socket to enter
     * a ready to read, write or error condition state.
     *
     * @param w Specifies the desired socket state.
     *
     * @par Example
     * Waiting for a socket to become readable.
     * @code
     * ip::tcp::socket socket();
     * ...
     * socket.wait(ip::tcp::socket::wait_read);
     * @endcode
     */
    void wait(wait_type w)
    {
        std::error_code ec;
        this->wait(w, ec);
        throw_if(ec, "wait");
    }

    /**
     * Wait for the socket to become ready to read, ready to write, or to have
     * pending error conditions.
     * This function is used to perform a blocking wait for a socket to enter
     * a ready to read, write or error condition state.
     *
     * @param w Specifies the desired socket state.
     *
     * @param ec Set to indicate what error occurred, if any.
     *
     * @par Example
     * Waiting for a socket to become readable.
     * @code
     * ip::tcp::socket socket();
     * ...
     * std::error_code ec;
     * socket.wait(ip::tcp::socket::wait_read, ec);
     * @endcode
     */
    std::error_code wait(wait_type w, std::error_code& ec)
    {
        // Check the status of the state
        int SelectStatus = 0;
        do 
        {
            switch (w)
            {
            case socket_base::wait_read:
                SelectStatus = socket_ops::poll_read(native_handle(), _state, 16,ec);
                break;
            case socket_base::wait_write:
                SelectStatus = socket_ops::poll_write(native_handle(), _state, 16, ec);
                break;
            case socket_base::wait_error:
                SelectStatus = socket_ops::poll_error(native_handle(), _state, 16, ec);
                break;
            default:
                ec = make_error_code(std::errc::invalid_argument);
                break;
            }
            if (SelectStatus < 0 || SelectStatus > 0)
            {
                break;
            }

        } while (SelectStatus == 0 );
        return ec;
    }

    /**
     * Disable sends or receives on the socket.
     * This function is used to disable send operations, receive operations, or
     * both.
     *
     * @param what Determines what types of operation will no longer be allowed.
     * @throws std::system_error Thrown on failure.
     *
     * @par Example
     * Shutting down the send side of the socket:
     * @code
     * ip::tcp::socket socket();
     * ...
     * socket.shutdown(ip::tcp::socket::shutdown_send);
     * @endcode
     */
    void shutdown(shutdown_type what)
    {
        std::error_code ec;
        this->shutdown(what, ec);
        throw_if(ec, "shutdown");
    }

    /**
     * Disable sends or receives on the socket.
     * This function is used to disable send operations, receive operations, or
     * both.
     *
     * @param what Determines what types of operation will no longer be allowed.
     *
     * @param ec Set to indicate what error occurred, if any.
     *
     * @par Example
     * Shutting down the send side of the socket:
     * @code
     * ip::tcp::socket socket();
     * ...
     * std::error_code ec;
     * socket.shutdown(ip::tcp::socket::shutdown_send, ec);
     * if (ec)
     * {
     *   // An error occurred.
     * }
     * @endcode
     */
    std::error_code shutdown(shutdown_type what, std::error_code& ec)
    {
        socket_ops::shutdown(native_handle(), what, ec);
        return ec ;
    }

    /**
     * Close the socket.
     * This function is used to close the socket. Any asynchronous send, receive
     * or connect operations will be cancelled immediately, and will complete
     * with the std::errc::operation_canceled error.
     *
     * @throws std::system_error Thrown on failure. Note that, even if
     * the function indicates an error, the underlying descriptor is closed.
     *
     * @note For portable behaviour with respect to graceful closure of a
     * connected socket, call shutdown() before closing the socket.
     */
    void close()
    {
        std::error_code ec;
        this->close(ec);
        throw_if(ec, "close");
    }

    /**
     * Close the socket.
     * This function is used to close the socket. Any asynchronous send, receive
     * or connect operations will be cancelled immediately, and will complete
     * with the std::errc::operation_aborted error.
     *
     * @param ec Set to indicate what error occurred, if any. Note that, even if
     * the function indicates an error, the underlying descriptor is closed.
     *
     * @par Example
     * @code
     * ip::tcp::socket socket();
     * ...
     * std::error_code ec;
     * socket.close(ec);
     * if (ec)
     * {
     *   // An error occurred.
     * }
     * @endcode
     *
     * @note For portable behaviour with respect to graceful closure of a
     * connected socket, call shutdown() before closing the socket.
     */
    std::error_code close(std::error_code& ec)
    {
        socket_ops::close(native_handle(), _state, false, ec);
        reset();
        return ec;
    }

    /**
     * Get the local endpoint of the socket.
     * This function is used to obtain the locally bound endpoint of the socket.
     *
     * @returns An object that represents the local endpoint of the socket.
     *
     * @throws std::system_error Thrown on failure.
     *
     * @par Example
     * @code
     * ip::tcp::socket socket();
     * ...
     * ip::tcp::endpoint endpoint = socket.local_endpoint();
     * @endcode
     */
    endpoint_type local_endpoint() const
    {
        std::error_code ec;
        endpoint_type endpoint = this->local_endpoint(ec);
        throw_if(ec, "local_endpoint");
        return endpoint;
    }

    /**
     * Get the local endpoint of the socket.
     * This function is used to obtain the locally bound endpoint of the socket.
     *
     * @param ec Set to indicate what error occurred, if any.
     *
     * @returns An object that represents the local endpoint of the socket.
     * Returns a default-constructed endpoint object if an error occurred.
     *
     * @par Example
     * @code
     * ip::tcp::socket socket();
     * ...
     * std::error_code ec;
     * ip::tcp::endpoint endpoint = socket.local_endpoint(ec);
     * if (ec)
     * {
     *   // An error occurred.
     * }
     * @endcode
     */
    endpoint_type local_endpoint(std::error_code& ec) const
    {
        endpoint_type endpoint;
        int addr_len = static_cast<int>(endpoint.capacity());
        if (socket_ops::getsockname(native_handle(), endpoint.data(), &addr_len, ec))
            return endpoint_type();
        endpoint.resize(addr_len);
        return endpoint;
    }


    /**
     * Get the remote endpoint of the socket.
     * This function is used to obtain the remote endpoint of the socket.
     *
     * @returns An object that represents the remote endpoint of the socket.
     *
     * @throws std::system_error Thrown on failure.
     *
     * @par Example
     * @code
     * ip::tcp::socket socket();
     * ...
     * ip::tcp::endpoint endpoint = socket.remote_endpoint();
     * @endcode
     */
    endpoint_type remote_endpoint() const
    {
        std::error_code ec;
        endpoint_type endpoint = this->remote_endpoint(ec);
        throw_if(ec, "remote_endpoint");
        return endpoint;
    }

    /**
     * Get the remote endpoint of the socket.
     * Get the remote endpoint of the socket.
     * This function is used to obtain the remote endpoint of the socket.
     *
     * @param ec Set to indicate what error occurred, if any.
     *
     * @returns An object that represents the remote endpoint of the socket.
     * Returns a default-constructed endpoint object if an error occurred.
     *
     * @par Example
     * @code
     * ip::tcp::socket socket();
     * ...
     * std::error_code ec;
     * ip::tcp::endpoint endpoint = socket.remote_endpoint(ec);
     * if (ec)
     * {
     *   // An error occurred.
     * }
     * @endcode
     */
    endpoint_type remote_endpoint(std::error_code& ec) const
    {
        endpoint_type endpoint;
        std::size_t addr_len = endpoint.capacity();
        if (socket_ops::getpeername(native_handle(), endpoint.data(), &addr_len, false, ec))
            return endpoint_type();
        endpoint.resize(addr_len);
        return endpoint;
    }

    native_handle_type native_handle()const
    {
        if (!_shared_socket)
        {
            return invalid_socket;
        }
        return *_shared_socket;
    }

    explicit operator bool() const
    {
        return (_shared_socket && *_shared_socket != invalid_socket);
    }

    /**
     * Set an option on the socket.
     * This function is used to set an option on the socket.
     *
     * @param option The new option value to be set on the socket.
     *
     * @throws std::system_error Thrown on failure.
     *
     * @sa SettableSocketOption
     * NetLite::socket_base::broadcast
     * NetLite::socket_base::debug
     * NetLite::socket_base::do_not_route
     * NetLite::socket_base::keep_alive
     * NetLite::socket_base::send_buffer_size
     * NetLite::socket_base::receive_buffer_size
     * NetLite::socket_base::receive_low_watermark
     * NetLite::socket_base::send_low_watermark
     * NetLite::socket_base::reuse_address
     * NetLite::socket_base::linger
     * NetLite::ip::multicast::join_group
     * NetLite::ip::multicast::leave_group
     * NetLite::ip::multicast::outbound_interface
     * NetLite::ip::multicast::hops
     * NetLite::ip::multicast::enable_loopback
     * NetLite::ip::tcp::no_delay
     *
     * @par Example
     * Setting the IPPROTO_TCP/TCP_NODELAY option:
     * @code
     * NetLite::ip::tcp::socket socket(io_service);
     * ...
     * NetLite::ip::tcp::no_delay option(true);
     * socket.set_option(option);
     * @endcode
     */
    template <typename SettableSocketOption>
    void set_option(const SettableSocketOption& option)
    {
        std::error_code ec;
        this->set_option(option, ec);
        throw_if(ec, "set_option");
    }

    template <typename SettableSocketOption>
    std::error_code set_option(const SettableSocketOption& option,std::error_code& ec)
    {
        socket_ops::setsockopt(native_handle(), _state,
            option.level(_protocol), option.name(_protocol),
            option.data(_protocol), option.size(_protocol), ec);
        return ec;
    }

    /**
     * Get an option from the socket.
     * This function is used to get the current value of an option on the socket.
     *
     * @param option The option value to be obtained from the socket.
     *
     * @throws std::system_error Thrown on failure.
     *
     * @sa GettableSocketOption @n
     * NetLite::socket_base::broadcast
     * NetLite::socket_base::debug
     * NetLite::socket_base::do_not_route
     * NetLite::socket_base::keep_alive
     * NetLite::socket_base::send_buffer_size
     * NetLite::socket_base::receive_buffer_size
     * NetLite::socket_base::receive_low_watermark
     * NetLite::socket_base::send_low_watermark
     * NetLite::socket_base::reuse_address
     * NetLite::socket_base::linger
     * NetLite::ip::multicast::join_group
     * NetLite::ip::multicast::leave_group
     * NetLite::ip::multicast::outbound_interface
     * NetLite::ip::multicast::hops
     * NetLite::ip::multicast::enable_loopback
     * NetLite::ip::tcp::no_delay
     *
     * @par Example
     * Getting the value of the SOL_SOCKET/SO_KEEPALIVE option:
     * @code
     * NetLite::ip::tcp::socket socket(io_service);
     * ...
     * NetLite::ip::tcp::socket::keep_alive option;
     * socket.get_option(option);
     * bool is_set = option.value();
     * @endcode
     */
    template <typename GettableSocketOption>
    void get_option(GettableSocketOption& option) const
    {
        std::error_code ec;
        this->get_option(option, ec);
        throw_if(ec, "get_option");
    }

    template <typename GettableSocketOption>
    std::error_code get_option(GettableSocketOption& option, std::error_code& ec) const
    {
        std::size_t size = option.size(_protocol);
        socket_ops::getsockopt(native_handle(), _state,
            option.level(_protocol), option.name(_protocol),
            option.data(_protocol), &size, ec);
        if (!ec)
            option.resize(_protocol, size);
        return ec;
    }


    /**
     * Queries the socket to determine if there is a pending accept
     * @param ec Error code.
     * @return true if successful, false otherwise
     *
     * @par Example
     * @code
     * ip::tcp::socket socket();
     * ...
     * std::error_code ec;
     * socket.has_pending_accept(ec);
     * if (ec)
     * {
     *   // An error occurred.
     * }
     * @endcode
     */
    bool has_pending_accept(std::error_code& ec)
    {
        bool bHasPendingConnection = false;
        // make sure socket has no error state
        if (has_state(socket_state::haserror, ec) == state_return::No)
        {
            // get the read state
            state_return State = has_state(socket_state::readable, ec);
            // turn the result into the outputs
            bHasPendingConnection = State == state_return::Yes;
        }
        return bHasPendingConnection;
    }

protected:
    void holdsSocket(native_handle_type native_socket)
    {
        _shared_socket = shared_socket(new native_handle_type(native_socket));
    }

    void reset()
    {
        _shared_socket.reset();
        _open = false;
    }

    void move(basic_socket&& other)
    {
        this->_shared_socket = std::move(other._shared_socket);
        this->_open = other._open;
        this->_state = other._state;
        this->_protocol = other._protocol;
        other._open = false;
    }

    void copy(const basic_socket& other)
    {
        this->_shared_socket =other._shared_socket;
        this->_open = other._open;
        this->_state = other._state;
        this->_protocol = other._protocol;
    }

    state_return has_state(socket_state state, std::error_code& ec)
    {
        // Check the status of the state
        int32 SelectStatus = 0;
        switch (state)
        {
        case socket_state::readable:
            SelectStatus = socket_ops::poll_read(native_handle(), _state, ec);
            break;

        case socket_state::writable:
            SelectStatus = socket_ops::poll_write(native_handle(), _state, ec);
            break;

        case socket_state::haserror:
            SelectStatus = socket_ops::poll_error(native_handle(), _state, ec);
            break;
        }

        // if the select returns a positive number,
        // the socket had the state, 
        // 0 means didn't have it, 
        // and negative is API error condition (not socket's error state)
        return SelectStatus > 0 ? state_return::Yes :(SelectStatus == 0 ? state_return::No : state_return::EncounteredError);
    }
private:

    /// Holds the BSD socket object. */
    shared_socket           _shared_socket;

    /// The socket state type
    socket_ops::state_type  _state;

    /// The ip address type (ipv4/ipv6).
    Protocol                _protocol;

    /// The socket is open ?
    bool                    _open;
};

} // namespace NetLite

#endif // END OF NETLITE_BASIC_SOCKET_HPP
