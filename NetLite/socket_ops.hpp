/****************************************************************************
  Copyright (c) 2018 libo All rights reserved.
  
  losemymind.libo@gmail.com

****************************************************************************/
#ifndef NETLITE_SOCKET_OPS_HPP
#define NETLITE_SOCKET_OPS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include "NetLite/config.hpp"
#include "NetLite/net_error_code.hpp"
#include "NetLite/socket_types.hpp"


namespace NetLite {
namespace socket_ops {

// Socket state bits.
enum
{
  // The user wants a non-blocking socket.
  user_set_non_blocking = 1,

  // The socket has been set non-blocking.
  internal_non_blocking = 2,

  // Helper "state" used to determine whether the socket is non-blocking.
  non_blocking = user_set_non_blocking | internal_non_blocking,

  // User wants connection_aborted errors, which are disabled by default.
  enable_connection_aborted = 4,

  // The user set the linger option. Needs to be checked when closing.
  user_set_linger = 8,

  // The socket is stream-oriented.
  stream_oriented = 16,

  // The socket is datagram-oriented.
  datagram_oriented = 32,

  // The socket may have been dup()-ed.
  possible_dup = 64
};

typedef unsigned char state_type;

struct noop_deleter { void operator()(void*) {} };
typedef std::shared_ptr<void> shared_cancel_token_type;
typedef std::weak_ptr<void> weak_cancel_token_type;


NETWORK_API socket_type accept(socket_type s, socket_addr_type* addr,
    std::size_t* addrlen, std::error_code& ec);

NETWORK_API socket_type sync_accept(socket_type s,
    state_type state, socket_addr_type* addr,
    std::size_t* addrlen, std::error_code& ec);

NETWORK_API bool non_blocking_accept(socket_type s,
    state_type state, socket_addr_type* addr, std::size_t* addrlen,
    std::error_code& ec, socket_type& new_socket);

NETWORK_API int bind(socket_type s, const socket_addr_type* addr,
    std::size_t addrlen, std::error_code& ec);

NETWORK_API int close(socket_type s, state_type& state,
    bool destruction, std::error_code& ec);

NETWORK_API bool set_user_non_blocking(socket_type s,
    state_type& state, bool value, std::error_code& ec);

NETWORK_API bool set_internal_non_blocking(socket_type s,
    state_type& state, bool value, std::error_code& ec);

NETWORK_API int shutdown(socket_type s,
    int what, std::error_code& ec);

NETWORK_API int connect(socket_type s, const socket_addr_type* addr,
    std::size_t addrlen, std::error_code& ec);

NETWORK_API void sync_connect(socket_type s, const socket_addr_type* addr,
    std::size_t addrlen, std::error_code& ec);

NETWORK_API bool non_blocking_connect(socket_type s,
    std::error_code& ec);

NETWORK_API int socketpair(int af, int type, int protocol,
    socket_type sv[2], std::error_code& ec);

NETWORK_API bool sockatmark(socket_type s, std::error_code& ec);

NETWORK_API size_t available(socket_type s, std::error_code& ec);

NETWORK_API int listen(socket_type s,
    int backlog, std::error_code& ec);

#if defined(_WIN32) || defined(__CYGWIN__)
typedef WSABUF buf;
#else // (PLATFORM_WINDOWS) || defined(__CYGWIN__)
typedef iovec buf;
#endif // (PLATFORM_WINDOWS) || defined(__CYGWIN__)

NETWORK_API void init_buf(buf& b, void* data, size_t size);

NETWORK_API void init_buf(buf& b, const void* data, size_t size);

NETWORK_API signed_size_type recv(socket_type s, buf* bufs,
    size_t count, int flags, std::error_code& ec);

NETWORK_API size_t sync_recv(socket_type s, state_type state, buf* bufs,
    size_t count, int flags, bool all_empty, std::error_code& ec);

NETWORK_API bool non_blocking_recv(socket_type s,
    buf* bufs, size_t count, int flags, bool is_stream,
    std::error_code& ec, size_t& bytes_transferred);

NETWORK_API signed_size_type recvfrom(socket_type s, buf* bufs,
    size_t count, int flags, socket_addr_type* addr,
    std::size_t* addrlen, std::error_code& ec);

NETWORK_API size_t sync_recvfrom(socket_type s, state_type state,
    buf* bufs, size_t count, int flags, socket_addr_type* addr,
    std::size_t* addrlen, std::error_code& ec);

NETWORK_API bool non_blocking_recvfrom(socket_type s,
    buf* bufs, size_t count, int flags,
    socket_addr_type* addr, std::size_t* addrlen,
    std::error_code& ec, size_t& bytes_transferred);

NETWORK_API signed_size_type recvmsg(socket_type s, buf* bufs,
    size_t count, int in_flags, int& out_flags,
    std::error_code& ec);

NETWORK_API size_t sync_recvmsg(socket_type s, state_type state,
    buf* bufs, size_t count, int in_flags, int& out_flags,
    std::error_code& ec);

NETWORK_API bool non_blocking_recvmsg(socket_type s,
    buf* bufs, size_t count, int in_flags, int& out_flags,
    std::error_code& ec, size_t& bytes_transferred);


NETWORK_API signed_size_type send(socket_type s, const buf* bufs,
    size_t count, int flags, std::error_code& ec);

NETWORK_API size_t sync_send(socket_type s, state_type state,
    const buf* bufs, size_t count, int flags,
    bool all_empty, std::error_code& ec);

NETWORK_API bool non_blocking_send(socket_type s,
    const buf* bufs, size_t count, int flags,
    std::error_code& ec, size_t& bytes_transferred);

NETWORK_API signed_size_type sendto(socket_type s, const buf* bufs,
    size_t count, int flags, const socket_addr_type* addr,
    std::size_t addrlen, std::error_code& ec);

NETWORK_API size_t sync_sendto(socket_type s, state_type state,
    const buf* bufs, size_t count, int flags, const socket_addr_type* addr,
    std::size_t addrlen, std::error_code& ec);

NETWORK_API bool non_blocking_sendto(socket_type s,
    const buf* bufs, size_t count, int flags,
    const socket_addr_type* addr, std::size_t addrlen,
    std::error_code& ec, size_t& bytes_transferred);

NETWORK_API socket_type socket(int af, int type, int protocol,
    std::error_code& ec);

NETWORK_API int setsockopt(socket_type s, state_type& state,
    int level, int optname, const void* optval,
    std::size_t optlen, std::error_code& ec);

NETWORK_API int getsockopt(socket_type s, state_type state,
    int level, int optname, void* optval,
    size_t* optlen, std::error_code& ec);

NETWORK_API int getpeername(socket_type s, socket_addr_type* addr,
    std::size_t* addrlen, bool cached, std::error_code& ec);

NETWORK_API int getsockname(socket_type s, socket_addr_type* addr,
    std::size_t* addrlen, std::error_code& ec);

NETWORK_API int ioctl(socket_type s, state_type& state,
    int cmd, ioctl_arg_type* arg, std::error_code& ec);

NETWORK_API int select(int nfds, fd_set* readfds, fd_set* writefds,
    fd_set* exceptfds, timeval* timeout, std::error_code& ec);

NETWORK_API int poll_read(socket_type s,
    state_type state, int msec, std::error_code& ec);

NETWORK_API int poll_write(socket_type s,
    state_type state, int msec, std::error_code& ec);

NETWORK_API int poll_error(socket_type s,
    state_type state, int msec, std::error_code& ec);

NETWORK_API int poll_connect(socket_type s,
    int msec, std::error_code& ec);

NETWORK_API const char* inet_ntop(int af, const void* src, char* dest,
    size_t length, unsigned long scope_id, std::error_code& ec);

NETWORK_API int inet_pton(int af, const char* src, void* dest,
    unsigned long* scope_id, std::error_code& ec);

NETWORK_API int gethostname(char* name,
    int namelen, std::error_code& ec);

NETWORK_API std::error_code getaddrinfo(const char* host,
    const char* service, const addrinfo_type& hints,
    addrinfo_type** result, std::error_code& ec);

NETWORK_API std::error_code background_getaddrinfo(
    const weak_cancel_token_type& cancel_token, const char* host,
    const char* service, const addrinfo_type& hints,
    addrinfo_type** result, std::error_code& ec);

NETWORK_API void freeaddrinfo(addrinfo_type* ai);

NETWORK_API std::error_code getnameinfo(
    const socket_addr_type* addr, std::size_t addrlen,
    char* host, std::size_t hostlen, char* serv,
    std::size_t servlen, int flags, std::error_code& ec);

NETWORK_API std::error_code sync_getnameinfo(
    const socket_addr_type* addr, std::size_t addrlen,
    char* host, std::size_t hostlen, char* serv,
    std::size_t servlen, int sock_type, std::error_code& ec);

NETWORK_API std::error_code background_getnameinfo(
    const weak_cancel_token_type& cancel_token,
    const socket_addr_type* addr, std::size_t addrlen,
    char* host, std::size_t hostlen, char* serv,
    std::size_t servlen, int sock_type, std::error_code& ec);

NETWORK_API u_long_type network_to_host_long(u_long_type value);

NETWORK_API u_long_type host_to_network_long(u_long_type value);

NETWORK_API u_short_type network_to_host_short(u_short_type value);

NETWORK_API u_short_type host_to_network_short(u_short_type value);

} // namespace socket_ops
} // namespace NetLite


# include "NetLite/socket_ops.ipp"

#endif // END OF NETLITE_SOCKET_OPS_HPP
