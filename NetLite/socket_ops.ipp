
#ifndef NETLITE_SOCKET_OPS_IPP
#define NETLITE_SOCKET_OPS_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <new>
#include "NetLite/socket_ops.hpp"
#include "NetLite/net_error_code.hpp"
#include "NetLite/socket_types.hpp"


namespace NetLite {
namespace socket_ops {

#if defined(_WIN32) || defined(__CYGWIN__)
struct msghdr { int msg_namelen; };
#endif // (defined(_WIN32)) || defined(__CYGWIN__)

#if defined(__hpux)
// HP-UX doesn't declare these functions extern "C", so they are declared again
// here to avoid linker errors about undefined symbols.
extern "C" char* if_indextoname(unsigned int, char*);
extern "C" unsigned int if_nametoindex(const char*);
#endif // defined(__hpux)

inline void clear_last_error()
{
#if defined(_WIN32) || defined(__CYGWIN__)
  WSASetLastError(0);
#else
  errno = 0;
#endif
}

inline int get_error_code()
{
#if defined(_WIN32) || defined(__CYGWIN__)
    return WinErrorCodeToErrc(WSAGetLastError());
#else
    return errno;
#endif
}

template <typename ReturnType>
inline ReturnType error_wrapper(ReturnType return_value, std::error_code& ec)
{
    ec = std::error_code(get_error_code(), std::generic_category());
    return return_value;
}

template <typename SockLenType>
inline socket_type call_accept(SockLenType msghdr::*,
    socket_type s, socket_addr_type* addr, std::size_t* addrlen)
{
  SockLenType tmp_addrlen = addrlen ? (SockLenType)*addrlen : 0;
  socket_type result = ::accept(s, addr, addrlen ? &tmp_addrlen : 0);
  if (addrlen)
    *addrlen = (std::size_t)tmp_addrlen;
  return result;
}

socket_type accept(socket_type s, socket_addr_type* addr,
    std::size_t* addrlen, std::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = std::make_error_code(std::errc::bad_file_descriptor);
    return invalid_socket;
  }

  clear_last_error();

  socket_type new_s = error_wrapper(call_accept(
        &msghdr::msg_namelen, s, addr, addrlen), ec);
  if (new_s == invalid_socket)
    return new_s;

#if defined(__MACH__) && defined(__APPLE__) || defined(__FreeBSD__)
  int optval = 1;
  int result = error_wrapper(::setsockopt(new_s,
        SOL_SOCKET, SO_NOSIGPIPE, &optval, sizeof(optval)), ec);
  if (result != 0)
  {
    ::close(new_s);
    return invalid_socket;
  }
#endif

  ec = std::error_code();
  return new_s;
}

socket_type sync_accept(socket_type s, state_type state,
    socket_addr_type* addr, std::size_t* addrlen, std::error_code& ec)
{
  // Accept a socket.
  for (;;)
  {
    // Try to complete the operation without blocking.
    socket_type new_socket = socket_ops::accept(s, addr, addrlen, ec);

    // Check if operation succeeded.
    if (new_socket != invalid_socket)
      return new_socket;

    // Operation failed.
    if (ec == std::errc::operation_would_block
        || ec == std::errc::resource_unavailable_try_again)
    {
      if (state & user_set_non_blocking)
        break;
      // Fall through to retry operation.
    }
    else if (ec == std::errc::connection_aborted)
    {
      if (state & enable_connection_aborted)
        break;
      // Fall through to retry operation.
    }
#if defined(EPROTO)
    else if (ec.value() == EPROTO)
    {
      if (state & enable_connection_aborted)
        break;
      // Fall through to retry operation.
    }
#endif // defined(EPROTO)
    else
      break;

    // Wait for socket to become ready.
    if (socket_ops::poll_read(s, 0, -1, ec) < 0)
      break;
  }
  return invalid_socket;
}

bool non_blocking_accept(socket_type s,
    state_type state, socket_addr_type* addr, std::size_t* addrlen,
    std::error_code& ec, socket_type& new_socket)
{
  for (;;)
  {
    // Accept the waiting connection.
    new_socket = socket_ops::accept(s, addr, addrlen, ec);

    // Check if operation succeeded.
    if (new_socket != invalid_socket)
      return true;

    // Retry operation if interrupted by signal.
    if (ec == std::errc::interrupted)
      continue;

    // Operation failed.
    if (ec == std::errc::operation_would_block
        || ec == std::errc::resource_unavailable_try_again)
    {
      // Fall through to retry operation.
    }
    else if (ec == std::errc::connection_aborted)
    {
      if (state & enable_connection_aborted)
        return true;
      // Fall through to retry operation.
    }
#if defined(EPROTO)
    else if (ec.value() == EPROTO)
    {
      if (state & enable_connection_aborted)
        return true;
      // Fall through to retry operation.
    }
#endif // defined(EPROTO)
    else
      return true;

    return false;
  }
}

template <typename SockLenType>
inline int call_bind(SockLenType msghdr::*,
    socket_type s, const socket_addr_type* addr, std::size_t addrlen)
{
  return ::bind(s, addr, (SockLenType)addrlen);
}

int bind(socket_type s, const socket_addr_type* addr,
    std::size_t addrlen, std::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = std::make_error_code(std::errc::bad_file_descriptor);
    return socket_error_retval;
  }

  clear_last_error();
  int result = error_wrapper(call_bind(
        &msghdr::msg_namelen, s, addr, addrlen), ec);
  if (result == 0)
    ec = std::error_code();
  return result;
}

int close(socket_type s, state_type& state,
    bool destruction, std::error_code& ec)
{
  int result = 0;
  if (s != invalid_socket)
  {
    // We don't want the destructor to block, so set the socket to linger in
    // the background. If the user doesn't like this behaviour then they need
    // to explicitly close the socket.
    if (destruction && (state & user_set_linger))
    {
      ::linger opt;
      opt.l_onoff = 0;
      opt.l_linger = 0;
      std::error_code ignored_ec;
      socket_ops::setsockopt(s, state, SOL_SOCKET,
          SO_LINGER, &opt, sizeof(opt), ignored_ec);
    }

    clear_last_error();
#if defined(_WIN32) || defined(__CYGWIN__)
    result = error_wrapper(::closesocket(s), ec);
#else // defined(_WIN32) || defined(__CYGWIN__)
    result = error_wrapper(::close(s), ec);
#endif // defined(_WIN32) || defined(__CYGWIN__)

    if (result != 0
        && (ec == std::errc::operation_would_block
          || ec == std::errc::resource_unavailable_try_again))
    {
      // According to UNIX Network Programming Vol. 1, it is possible for
      // close() to fail with EWOULDBLOCK under certain circumstances. What
      // isn't clear is the state of the descriptor after this error. The one
      // current OS where this behaviour is seen, Windows, says that the socket
      // remains open. Therefore we'll put the descriptor back into blocking
      // mode and have another attempt at closing it.
#if defined(_WIN32) || defined(__CYGWIN__)
      ioctl_arg_type arg = 0;
      ::ioctlsocket(s, FIONBIO, &arg);
#else // defined(_WIN32) || defined(__CYGWIN__)
# if defined(__SYMBIAN32__)
      int flags = ::fcntl(s, F_GETFL, 0);
      if (flags >= 0)
        ::fcntl(s, F_SETFL, flags & ~O_NONBLOCK);
# else // defined(__SYMBIAN32__)
      ioctl_arg_type arg = 0;
      ::ioctl(s, FIONBIO, &arg);
# endif // defined(__SYMBIAN32__)
#endif // defined(_WIN32) || defined(__CYGWIN__)
      state &= ~non_blocking;

      clear_last_error();
#if defined(_WIN32) || defined(__CYGWIN__)
      result = error_wrapper(::closesocket(s), ec);
#else // defined(_WIN32) || defined(__CYGWIN__)
      result = error_wrapper(::close(s), ec);
#endif // defined(_WIN32) || defined(__CYGWIN__)
    }
  }

  if (result == 0)
    ec = std::error_code();
  return result;
}

bool set_user_non_blocking(socket_type s,
    state_type& state, bool value, std::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = std::make_error_code(std::errc::bad_file_descriptor);
    return false;
  }

  clear_last_error();
#if defined(_WIN32) || defined(__CYGWIN__)
  ioctl_arg_type arg = (value ? 1 : 0);
  int result = error_wrapper(::ioctlsocket(s, FIONBIO, &arg), ec);
#elif defined(__SYMBIAN32__)
  int result = error_wrapper(::fcntl(s, F_GETFL, 0), ec);
  if (result >= 0)
  {
    clear_last_error();
    int flag = (value ? (result | O_NONBLOCK) : (result & ~O_NONBLOCK));
    result = error_wrapper(::fcntl(s, F_SETFL, flag), ec);
  }
#else
  ioctl_arg_type arg = (value ? 1 : 0);
  int result = error_wrapper(::ioctl(s, FIONBIO, &arg), ec);
#endif

  if (result >= 0)
  {
    ec = std::error_code();
    if (value)
      state |= user_set_non_blocking;
    else
    {
      // Clearing the user-set non-blocking mode always overrides any
      // internally-set non-blocking flag. Any subsequent asynchronous
      // operations will need to re-enable non-blocking I/O.
      state &= ~(user_set_non_blocking | internal_non_blocking);
    }
    return true;
  }

  return false;
}

bool set_internal_non_blocking(socket_type s,
    state_type& state, bool value, std::error_code& ec)
{
  if (s == invalid_socket)
  {
      
    ec = std::make_error_code(std::errc::bad_file_descriptor);
    return false;
  }

  if (!value && (state & user_set_non_blocking))
  {
    // It does not make sense to clear the internal non-blocking flag if the
    // user still wants non-blocking behaviour. Return an error and let the
    // caller figure out whether to update the user-set non-blocking flag.
    ec = std::make_error_code(std::errc::invalid_argument);
    return false;
  }

  clear_last_error();
#if defined(_WIN32) || defined(__CYGWIN__)
  ioctl_arg_type arg = (value ? 1 : 0);
  int result = error_wrapper(::ioctlsocket(s, FIONBIO, &arg), ec);
#elif defined(__SYMBIAN32__)
  int result = error_wrapper(::fcntl(s, F_GETFL, 0), ec);
  if (result >= 0)
  {
    clear_last_error();
    int flag = (value ? (result | O_NONBLOCK) : (result & ~O_NONBLOCK));
    result = error_wrapper(::fcntl(s, F_SETFL, flag), ec);
  }
#else
  ioctl_arg_type arg = (value ? 1 : 0);
  int result = error_wrapper(::ioctl(s, FIONBIO, &arg), ec);
#endif

  if (result >= 0)
  {
    ec = std::error_code();
    if (value)
      state |= internal_non_blocking;
    else
      state &= ~internal_non_blocking;
    return true;
  }

  return false;
}

int shutdown(socket_type s, int what, std::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = std::make_error_code(std::errc::bad_file_descriptor);
    return socket_error_retval;
  }

  clear_last_error();
  int result = error_wrapper(::shutdown(s, what), ec);
  if (result == 0)
    ec = std::error_code();
  return result;
}

template <typename SockLenType>
inline int call_connect(SockLenType msghdr::*,
    socket_type s, const socket_addr_type* addr, std::size_t addrlen)
{
  return ::connect(s, addr, (SockLenType)addrlen);
}

int connect(socket_type s, const socket_addr_type* addr,
    std::size_t addrlen, std::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = std::make_error_code(std::errc::bad_file_descriptor);
    return socket_error_retval;
  }

  clear_last_error();
  int result = error_wrapper(call_connect(
        &msghdr::msg_namelen, s, addr, addrlen), ec);
  if (result == 0)
    ec = std::error_code();
#if defined(__linux__)
  else if (ec == std::errc::resource_unavailable_try_again)
    ec = std::error::no_buffer_space;
#endif // defined(__linux__)
  return result;
}

void sync_connect(socket_type s, const socket_addr_type* addr,
    std::size_t addrlen, std::error_code& ec)
{
  // Perform the connect operation.
  socket_ops::connect(s, addr, addrlen, ec);
  if (ec != std::errc::operation_in_progress
      && ec != std::errc::operation_would_block)
  {
    // The connect operation finished immediately.
    return;
  }

  // Wait for socket to become ready.
  if (socket_ops::poll_connect(s, -1, ec) < 0)
    return;

  // Get the error code from the connect operation.
  int connect_error = 0;
  size_t connect_error_len = sizeof(connect_error);
  if (socket_ops::getsockopt(s, 0, SOL_SOCKET, SO_ERROR,
        &connect_error, &connect_error_len, ec) == socket_error_retval)
    return;

  // Return the result of the connect operation.
  ec = std::error_code(connect_error, std::generic_category());
}

bool non_blocking_connect(socket_type s, std::error_code& ec)
{
  // Check if the connect operation has finished. This is required since we may
  // get spurious readiness notifications from the reactor.
#if defined(_WIN32) \
  || defined(__CYGWIN__) \
  || defined(__SYMBIAN32__)
  fd_set write_fds;
  FD_ZERO(&write_fds);
  FD_SET(s, &write_fds);
  fd_set except_fds;
  FD_ZERO(&except_fds);
  FD_SET(s, &except_fds);
  timeval zero_timeout;
  zero_timeout.tv_sec = 0;
  zero_timeout.tv_usec = 0;
  int ready = ::select(s + 1, 0, &write_fds, &except_fds, &zero_timeout);
#else // defined(_WIN32)
      // || defined(__CYGWIN__)
      // || defined(__SYMBIAN32__)
  pollfd fds;
  fds.fd = s;
  fds.events = POLLOUT;
  fds.revents = 0;
  int ready = ::poll(&fds, 1, 0);
#endif // defined(_WIN32)
       // || defined(__CYGWIN__)
       // || defined(__SYMBIAN32__)
  if (ready == 0)
  {
    // The asynchronous connect operation is still in progress.
    return false;
  }

  // Get the error code from the connect operation.
  int connect_error = 0;
  size_t connect_error_len = sizeof(connect_error);
  if (socket_ops::getsockopt(s, 0, SOL_SOCKET, SO_ERROR,
        &connect_error, &connect_error_len, ec) == 0)
  {
    if (connect_error)
    {
        ec = std::error_code(connect_error, std::generic_category());
    }
    else
      ec = std::error_code();
  }

  return true;
}

int socketpair(int af, int type, int protocol,
    socket_type sv[2], std::error_code& ec)
{
#if defined(_WIN32) || defined(__CYGWIN__)
  (void)(af);
  (void)(type);
  (void)(protocol);
  (void)(sv);
  ec = std::make_error_code(std::errc::operation_not_supported);
  return socket_error_retval;
#else
  clear_last_error();
  int result = error_wrapper(::socketpair(af, type, protocol, sv), ec);
  if (result == 0)
    ec = std::error_code();
  return result;
#endif
}

bool sockatmark(socket_type s, std::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = std::make_error_code(std::errc::bad_file_descriptor);
    return false;
  }

#if defined(SIOCATMARK)
  ioctl_arg_type value = 0;
# if defined(_WIN32) || defined(__CYGWIN__)
  int result = error_wrapper(::ioctlsocket(s, SIOCATMARK, &value), ec);
# else // defined(_WIN32) || defined(__CYGWIN__)
  int result = error_wrapper(::ioctl(s, SIOCATMARK, &value), ec);
# endif // defined(_WIN32) || defined(__CYGWIN__)
  if (result == 0)
    ec = std::error_code();
# if defined(ENOTTY)
  if (ec.value() == ENOTTY)
    ec = std::make_error_code(std::errc::not_a_socket);
# endif // defined(ENOTTY)
#else // defined(SIOCATMARK)
  int value = error_wrapper(::sockatmark(s), ec);
  if (value != -1)
    ec = std::error_code();
#endif // defined(SIOCATMARK)

  return ec ? false : value != 0;
}

size_t available(socket_type s, std::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = std::make_error_code(std::errc::bad_file_descriptor);
    return 0;
  }

  ioctl_arg_type value = 0;
#if defined(_WIN32) || defined(__CYGWIN__)
  int result = error_wrapper(::ioctlsocket(s, FIONREAD, &value), ec);
#else // defined(_WIN32) || defined(__CYGWIN__)
  int result = error_wrapper(::ioctl(s, FIONREAD, &value), ec);
#endif // defined(_WIN32) || defined(__CYGWIN__)
  if (result == 0)
    ec = std::error_code();
#if defined(ENOTTY)
  if (ec.value() == ENOTTY)
    ec = std::make_error_code(std::errc::not_a_socket);
#endif // defined(ENOTTY)

  return ec ? static_cast<size_t>(0) : static_cast<size_t>(value);
}

int listen(socket_type s, int backlog, std::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = std::make_error_code(std::errc::bad_file_descriptor);
    return socket_error_retval;
  }

  clear_last_error();
  int result = error_wrapper(::listen(s, backlog), ec);
  if (result == 0)
    ec = std::error_code();
  return result;
}

inline void init_buf_iov_base(void*& base, void* addr)
{
  base = addr;
}

template <typename T>
inline void init_buf_iov_base(T& base, void* addr)
{
  base = static_cast<T>(addr);
}

#if defined(_WIN32) || defined(__CYGWIN__)
typedef WSABUF buf;
#else // defined(_WIN32) || defined(__CYGWIN__)
typedef iovec buf;
#endif // defined(_WIN32) || defined(__CYGWIN__)

void init_buf(buf& b, void* data, size_t size)
{
#if defined(_WIN32) || defined(__CYGWIN__)
  b.buf = static_cast<char*>(data);
  b.len = static_cast<ULONG>(size);
#else // defined(_WIN32) || defined(__CYGWIN__)
  init_buf_iov_base(b.iov_base, data);
  b.iov_len = size;
#endif // defined(_WIN32) || defined(__CYGWIN__)
}

void init_buf(buf& b, const void* data, size_t size)
{
#if defined(_WIN32) || defined(__CYGWIN__)
  b.buf = static_cast<char*>(const_cast<void*>(data));
  b.len = static_cast<u_long>(size);
#else // defined(_WIN32) || defined(__CYGWIN__)
  init_buf_iov_base(b.iov_base, const_cast<void*>(data));
  b.iov_len = size;
#endif // defined(_WIN32) || defined(__CYGWIN__)
}

inline void init_msghdr_msg_name(void*& name, socket_addr_type* addr)
{
  name = addr;
}

inline void init_msghdr_msg_name(void*& name, const socket_addr_type* addr)
{
  name = const_cast<socket_addr_type*>(addr);
}

template <typename T>
inline void init_msghdr_msg_name(T& name, socket_addr_type* addr)
{
  name = reinterpret_cast<T>(addr);
}

template <typename T>
inline void init_msghdr_msg_name(T& name, const socket_addr_type* addr)
{
  name = reinterpret_cast<T>(const_cast<socket_addr_type*>(addr));
}

signed_size_type recv(socket_type s, buf* bufs, size_t count,
    int flags, std::error_code& ec)
{
  clear_last_error();
#if defined(_WIN32) || defined(__CYGWIN__)
  // Receive some data.
  DWORD recv_buf_count = static_cast<DWORD>(count);
  DWORD bytes_transferred = 0;
  DWORD recv_flags = flags;
  int result = error_wrapper(::WSARecv(s, bufs,
        recv_buf_count, &bytes_transferred, &recv_flags, 0, 0), ec);
  if (ec.value() == ERROR_NETNAME_DELETED)
    ec = std::make_error_code(std::errc::connection_reset);
  else if (ec.value() == ERROR_PORT_UNREACHABLE)
    ec = std::make_error_code(std::errc::connection_refused);
  else if (ec.value() == WSAEMSGSIZE || ec.value() == ERROR_MORE_DATA)
    ec.assign(0, ec.category());
  if (result != 0)
    return socket_error_retval;
  ec = std::error_code();
  return bytes_transferred;
#else // defined(_WIN32) || defined(__CYGWIN__)
  msghdr msg = msghdr();
  msg.msg_iov = bufs;
  msg.msg_iovlen = static_cast<int>(count);
  signed_size_type result = error_wrapper(::recvmsg(s, &msg, flags), ec);
  if (result >= 0)
    ec = std::error_code();
  return result;
#endif // defined(_WIN32) || defined(__CYGWIN__)
}

size_t sync_recv(socket_type s, state_type state, buf* bufs,
    size_t count, int flags, bool all_empty, std::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = std::make_error_code(std::errc::bad_file_descriptor);
    return 0;
  }

  // A request to read 0 bytes on a stream is a no-op.
  if (all_empty && (state & stream_oriented))
  {
    ec = std::error_code();
    return 0;
  }

  // Read some data.
  for (;;)
  {
    // Try to complete the operation without blocking.
    signed_size_type bytes = socket_ops::recv(s, bufs, count, flags, ec);

    // Check if operation succeeded.
    if (bytes > 0)
      return bytes;

    // Check for EOF.
    if ((state & stream_oriented) && bytes == 0)
    {
      ec = std::make_error_code(std::errc::no_message_available);
      return 0;
    }

    // Operation failed.
    if ((state & user_set_non_blocking)
        || (ec != std::errc::operation_would_block
          && ec != std::errc::resource_unavailable_try_again))
      return 0;

    // Wait for socket to become ready.
    if (socket_ops::poll_read(s, 0, -1, ec) < 0)
      return 0;
  }
}

bool non_blocking_recv(socket_type s,
    buf* bufs, size_t count, int flags, bool is_stream,
    std::error_code& ec, size_t& bytes_transferred)
{
  for (;;)
  {
    // Read some data.
    signed_size_type bytes = socket_ops::recv(s, bufs, count, flags, ec);

    // Check for end of stream.
    if (is_stream && bytes == 0)
    {
      ec = std::make_error_code(std::errc::no_message_available);
      return true;
    }

    // Retry operation if interrupted by signal.
    if (ec == std::errc::interrupted)
      continue;

    // Check if we need to run the operation again.
    if (ec == std::errc::operation_would_block
        || ec == std::errc::resource_unavailable_try_again)
      return false;

    // Operation is complete.
    if (bytes >= 0)
    {
      ec = std::error_code();
      bytes_transferred = bytes;
    }
    else
      bytes_transferred = 0;

    return true;
  }
}


signed_size_type recvfrom(socket_type s, buf* bufs, size_t count,
    int flags, socket_addr_type* addr, std::size_t* addrlen,
    std::error_code& ec)
{
  clear_last_error();
#if defined(_WIN32) || defined(__CYGWIN__)
  // Receive some data.
  DWORD recv_buf_count = static_cast<DWORD>(count);
  DWORD bytes_transferred = 0;
  DWORD recv_flags = flags;
  int tmp_addrlen = (int)*addrlen;
  int result = error_wrapper(::WSARecvFrom(s, bufs, recv_buf_count,
        &bytes_transferred, &recv_flags, addr, &tmp_addrlen, 0, 0), ec);
  *addrlen = (std::size_t)tmp_addrlen;
  if (ec.value() == ERROR_NETNAME_DELETED)
    ec = std::make_error_code(std::errc::connection_reset);
  else if (ec.value() == ERROR_PORT_UNREACHABLE)
    ec = std::make_error_code(std::errc::connection_refused);
  else if (ec.value() == WSAEMSGSIZE || ec.value() == ERROR_MORE_DATA)
    ec.assign(0, ec.category());
  if (result != 0)
    return socket_error_retval;
  ec = std::error_code();
  return bytes_transferred;
#else // defined(_WIN32) || defined(__CYGWIN__)
  msghdr msg = msghdr();
  init_msghdr_msg_name(msg.msg_name, addr);
  msg.msg_namelen = static_cast<int>(*addrlen);
  msg.msg_iov = bufs;
  msg.msg_iovlen = static_cast<int>(count);
  signed_size_type result = error_wrapper(::recvmsg(s, &msg, flags), ec);
  *addrlen = msg.msg_namelen;
  if (result >= 0)
    ec = std::error_code();
  return result;
#endif // defined(_WIN32) || defined(__CYGWIN__)
}

size_t sync_recvfrom(socket_type s, state_type state, buf* bufs,
    size_t count, int flags, socket_addr_type* addr,
    std::size_t* addrlen, std::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = std::make_error_code(std::errc::bad_file_descriptor);
    return 0;
  }

  // Read some data.
  for (;;)
  {
    // Try to complete the operation without blocking.
    signed_size_type bytes = socket_ops::recvfrom(
        s, bufs, count, flags, addr, addrlen, ec);

    // Check if operation succeeded.
    if (bytes >= 0)
      return bytes;

    // Operation failed.
    if ((state & user_set_non_blocking)
        || (ec != std::errc::operation_would_block
          && ec != std::errc::resource_unavailable_try_again))
      return 0;

    // Wait for socket to become ready.
    if (socket_ops::poll_read(s, 0, -1, ec) < 0)
      return 0;
  }
}

bool non_blocking_recvfrom(socket_type s,
    buf* bufs, size_t count, int flags,
    socket_addr_type* addr, std::size_t* addrlen,
    std::error_code& ec, size_t& bytes_transferred)
{
  for (;;)
  {
    // Read some data.
    signed_size_type bytes = socket_ops::recvfrom(
        s, bufs, count, flags, addr, addrlen, ec);

    // Retry operation if interrupted by signal.
    if (ec == std::errc::interrupted)
      continue;

    // Check if we need to run the operation again.
    if (ec == std::errc::operation_would_block
        || ec == std::errc::resource_unavailable_try_again)
      return false;

    // Operation is complete.
    if (bytes >= 0)
    {
      ec = std::error_code();
      bytes_transferred = bytes;
    }
    else
      bytes_transferred = 0;

    return true;
  }
}

signed_size_type recvmsg(socket_type s, buf* bufs, size_t count,
    int in_flags, int& out_flags, std::error_code& ec)
{
  clear_last_error();
#if defined(_WIN32) || defined(__CYGWIN__)
  out_flags = 0;
  return socket_ops::recv(s, bufs, count, in_flags, ec);
#else // defined(_WIN32) || defined(__CYGWIN__)
  msghdr msg = msghdr();
  msg.msg_iov = bufs;
  msg.msg_iovlen = static_cast<int>(count);
  signed_size_type result = error_wrapper(::recvmsg(s, &msg, in_flags), ec);
  if (result >= 0)
  {
    ec = std::error_code();
    out_flags = msg.msg_flags;
  }
  else
    out_flags = 0;
  return result;
#endif // defined(_WIN32) || defined(__CYGWIN__)
}

size_t sync_recvmsg(socket_type s, state_type state,
    buf* bufs, size_t count, int in_flags, int& out_flags,
    std::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = std::make_error_code(std::errc::bad_file_descriptor);
    return 0;
  }

  // Read some data.
  for (;;)
  {
    // Try to complete the operation without blocking.
    signed_size_type bytes = socket_ops::recvmsg(
        s, bufs, count, in_flags, out_flags, ec);

    // Check if operation succeeded.
    if (bytes >= 0)
      return bytes;

    // Operation failed.
    if ((state & user_set_non_blocking)
        || (ec != std::errc::operation_would_block
          && ec != std::errc::resource_unavailable_try_again))
      return 0;

    // Wait for socket to become ready.
    if (socket_ops::poll_read(s, 0, -1, ec) < 0)
      return 0;
  }
}

bool non_blocking_recvmsg(socket_type s,
    buf* bufs, size_t count, int in_flags, int& out_flags,
    std::error_code& ec, size_t& bytes_transferred)
{
  for (;;)
  {
    // Read some data.
    signed_size_type bytes = socket_ops::recvmsg(
        s, bufs, count, in_flags, out_flags, ec);

    // Retry operation if interrupted by signal.
    if (ec == std::errc::interrupted)
      continue;

    // Check if we need to run the operation again.
    if (ec == std::errc::operation_would_block
        || ec == std::errc::resource_unavailable_try_again)
      return false;

    // Operation is complete.
    if (bytes >= 0)
    {
      ec = std::error_code();
      bytes_transferred = bytes;
    }
    else
      bytes_transferred = 0;

    return true;
  }
}

signed_size_type send(socket_type s, const buf* bufs, size_t count,
    int flags, std::error_code& ec)
{
  clear_last_error();
#if defined(_WIN32) || defined(__CYGWIN__)
  // Send the data.
  DWORD send_buf_count = static_cast<DWORD>(count);
  DWORD bytes_transferred = 0;
  DWORD send_flags = flags;
  int result = error_wrapper(::WSASend(s, const_cast<buf*>(bufs),
        send_buf_count, &bytes_transferred, send_flags, 0, 0), ec);
  if (ec.value() == ERROR_NETNAME_DELETED)
    ec = std::make_error_code(std::errc::connection_reset);
  else if (ec.value() == ERROR_PORT_UNREACHABLE)
    ec = std::make_error_code(std::errc::connection_refused);
  if (result != 0)
    return socket_error_retval;
  ec = std::error_code();
  return bytes_transferred;
#else // defined(_WIN32) || defined(__CYGWIN__)
  msghdr msg = msghdr();
  msg.msg_iov = const_cast<buf*>(bufs);
  msg.msg_iovlen = static_cast<int>(count);
#if defined(__linux__)
  flags |= MSG_NOSIGNAL;
#endif // defined(__linux__)
  signed_size_type result = error_wrapper(::sendmsg(s, &msg, flags), ec);
  if (result >= 0)
    ec = std::error_code();
  return result;
#endif // defined(_WIN32) || defined(__CYGWIN__)
}

size_t sync_send(socket_type s, state_type state, const buf* bufs,
    size_t count, int flags, bool all_empty, std::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = std::make_error_code(std::errc::bad_file_descriptor);
    return 0;
  }

  // A request to write 0 bytes to a stream is a no-op.
  if (all_empty && (state & stream_oriented))
  {
    ec = std::error_code();
    return 0;
  }

  // Read some data.
  for (;;)
  {
    // Try to complete the operation without blocking.
    signed_size_type bytes = socket_ops::send(s, bufs, count, flags, ec);

    // Check if operation succeeded.
    if (bytes >= 0)
      return bytes;

    // Operation failed.
    if ((state & user_set_non_blocking)
        || (ec != std::errc::operation_would_block
          && ec != std::errc::resource_unavailable_try_again))
      return 0;

    // Wait for socket to become ready.
    if (socket_ops::poll_write(s, 0, -1, ec) < 0)
      return 0;
  }
}

bool non_blocking_send(socket_type s,
    const buf* bufs, size_t count, int flags,
    std::error_code& ec, size_t& bytes_transferred)
{
  for (;;)
  {
    // Write some data.
    signed_size_type bytes = socket_ops::send(s, bufs, count, flags, ec);

    // Retry operation if interrupted by signal.
    if (ec == std::errc::interrupted)
      continue;

    // Check if we need to run the operation again.
    if (ec == std::errc::operation_would_block
        || ec == std::errc::resource_unavailable_try_again)
      return false;

    // Operation is complete.
    if (bytes >= 0)
    {
      ec = std::error_code();
      bytes_transferred = bytes;
    }
    else
      bytes_transferred = 0;

    return true;
  }
}

signed_size_type sendto(socket_type s, const buf* bufs, size_t count,
    int flags, const socket_addr_type* addr, std::size_t addrlen,
    std::error_code& ec)
{
  clear_last_error();
#if defined(_WIN32) || defined(__CYGWIN__)
  // Send the data.
  DWORD send_buf_count = static_cast<DWORD>(count);
  DWORD bytes_transferred = 0;
  int result = error_wrapper(::WSASendTo(s, const_cast<buf*>(bufs),
        send_buf_count, &bytes_transferred, flags, addr,
        static_cast<int>(addrlen), 0, 0), ec);
  if (ec.value() == ERROR_NETNAME_DELETED)
    ec = std::make_error_code(std::errc::connection_reset);
  else if (ec.value() == ERROR_PORT_UNREACHABLE)
    ec = std::make_error_code(std::errc::connection_refused);
  if (result != 0)
    return socket_error_retval;
  ec = std::error_code();
  return bytes_transferred;
#else // defined(_WIN32) || defined(__CYGWIN__)
  msghdr msg = msghdr();
  init_msghdr_msg_name(msg.msg_name, addr);
  msg.msg_namelen = static_cast<int>(addrlen);
  msg.msg_iov = const_cast<buf*>(bufs);
  msg.msg_iovlen = static_cast<int>(count);
#if defined(__linux__)
  flags |= MSG_NOSIGNAL;
#endif // defined(__linux__)
  signed_size_type result = error_wrapper(::sendmsg(s, &msg, flags), ec);
  if (result >= 0)
    ec = std::error_code();
  return result;
#endif // defined(_WIN32) || defined(__CYGWIN__)
}

size_t sync_sendto(socket_type s, state_type state, const buf* bufs,
    size_t count, int flags, const socket_addr_type* addr,
    std::size_t addrlen, std::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = std::make_error_code(std::errc::bad_file_descriptor);
    return 0;
  }

  // Write some data.
  for (;;)
  {
    // Try to complete the operation without blocking.
    signed_size_type bytes = socket_ops::sendto(
        s, bufs, count, flags, addr, addrlen, ec);

    // Check if operation succeeded.
    if (bytes >= 0)
      return bytes;

    // Operation failed.
    if ((state & user_set_non_blocking)
        || (ec != std::errc::operation_would_block
          && ec != std::errc::resource_unavailable_try_again))
      return 0;

    // Wait for socket to become ready.
    if (socket_ops::poll_write(s, 0, -1, ec) < 0)
      return 0;
  }
}

bool non_blocking_sendto(socket_type s,
    const buf* bufs, size_t count, int flags,
    const socket_addr_type* addr, std::size_t addrlen,
    std::error_code& ec, size_t& bytes_transferred)
{
  for (;;)
  {
    // Write some data.
    signed_size_type bytes = socket_ops::sendto(
        s, bufs, count, flags, addr, addrlen, ec);

    // Retry operation if interrupted by signal.
    if (ec == std::errc::interrupted)
      continue;

    // Check if we need to run the operation again.
    if (ec == std::errc::operation_would_block
        || ec == std::errc::resource_unavailable_try_again)
      return false;

    // Operation is complete.
    if (bytes >= 0)
    {
      ec = std::error_code();
      bytes_transferred = bytes;
    }
    else
      bytes_transferred = 0;

    return true;
  }
}

socket_type socket(int af, int type, int protocol,
    std::error_code& ec)
{
  clear_last_error();
#if defined(_WIN32) || defined(__CYGWIN__)
  socket_type s = error_wrapper(::WSASocketW(af, type, protocol, 0, 0,
        WSA_FLAG_OVERLAPPED), ec);
  if (s == invalid_socket)
    return s;

  if (af == NET_OS_DEF(AF_INET6))
  {
    // Try to enable the POSIX default behaviour of having IPV6_V6ONLY set to
    // false. This will only succeed on Windows Vista and later versions of
    // Windows, where a dual-stack IPv4/v6 implementation is available.
    DWORD optval = 0;
    ::setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY,
        reinterpret_cast<const char*>(&optval), sizeof(optval));
  }

  ec = std::error_code();

  return s;
#elif defined(__MACH__) && defined(__APPLE__) || defined(__FreeBSD__)
  socket_type s = error_wrapper(::socket(af, type, protocol), ec);
  if (s == invalid_socket)
    return s;

  int optval = 1;
  int result = error_wrapper(::setsockopt(s,
        SOL_SOCKET, SO_NOSIGPIPE, &optval, sizeof(optval)), ec);
  if (result != 0)
  {
    ::close(s);
    return invalid_socket;
  }

  return s;
#else
  int s = error_wrapper(::socket(af, type, protocol), ec);
  if (s >= 0)
    ec = std::error_code();
  return s;
#endif
}

template <typename SockLenType>
inline int call_setsockopt(SockLenType msghdr::*,
    socket_type s, int level, int optname,
    const void* optval, std::size_t optlen)
{
  return ::setsockopt(s, level, optname,
      (const char*)optval, (SockLenType)optlen);
}

int setsockopt(socket_type s, state_type& state, int level, int optname,
    const void* optval, std::size_t optlen, std::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = std::make_error_code(std::errc::bad_file_descriptor);
    return socket_error_retval;
  }

  if (level == custom_socket_option_level && optname == always_fail_option)
  {
    ec = std::make_error_code(std::errc::invalid_argument);
    return socket_error_retval;
  }

  if (level == custom_socket_option_level
      && optname == enable_connection_aborted_option)
  {
    if (optlen != sizeof(int))
    {
      ec = std::make_error_code(std::errc::invalid_argument);
      return socket_error_retval;
    }

    if (*static_cast<const int*>(optval))
      state |= enable_connection_aborted;
    else
      state &= ~enable_connection_aborted;
    ec = std::error_code();
    return 0;
  }

  if (level == SOL_SOCKET && optname == SO_LINGER)
    state |= user_set_linger;

#if defined(__BORLANDC__)
  // Mysteriously, using the getsockopt and setsockopt functions directly with
  // Borland C++ results in incorrect values being set and read. The bug can be
  // worked around by using function addresses resolved with GetProcAddress.
  if (HMODULE winsock_module = ::GetModuleHandleA("ws2_32"))
  {
    typedef int (WSAAPI *sso_t)(SOCKET, int, int, const char*, int);
    if (sso_t sso = (sso_t)::GetProcAddress(winsock_module, "setsockopt"))
    {
      clear_last_error();
      return error_wrapper(sso(s, level, optname,
            reinterpret_cast<const char*>(optval),
            static_cast<int>(optlen)), ec);
    }
  }
  ec = std::errc::bad_address;
  return socket_error_retval;
#else // defined(__BORLANDC__)
  clear_last_error();
  int result = error_wrapper(call_setsockopt(&msghdr::msg_namelen,
        s, level, optname, optval, optlen), ec);
  if (result == 0)
  {
    ec = std::error_code();

#if defined(__MACH__) && defined(__APPLE__) \
  || defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
    // To implement portable behaviour for SO_REUSEADDR with UDP sockets we
    // need to also set SO_REUSEPORT on BSD-based platforms.
    if ((state & datagram_oriented)
        && level == SOL_SOCKET && optname == SO_REUSEADDR)
    {
      call_setsockopt(&msghdr::msg_namelen, s,
          SOL_SOCKET, SO_REUSEPORT, optval, optlen);
    }
#endif
  }

  return result;
#endif // defined(__BORLANDC__)
}

template <typename SockLenType>
inline int call_getsockopt(SockLenType msghdr::*,
    socket_type s, int level, int optname,
    void* optval, std::size_t* optlen)
{
  SockLenType tmp_optlen = (SockLenType)*optlen;
  int result = ::getsockopt(s, level, optname, (char*)optval, &tmp_optlen);
  *optlen = (std::size_t)tmp_optlen;
  return result;
}

int getsockopt(socket_type s, state_type state, int level, int optname,
    void* optval, size_t* optlen, std::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = std::make_error_code(std::errc::bad_file_descriptor);
    return socket_error_retval;
  }

  if (level == custom_socket_option_level && optname == always_fail_option)
  {
    ec = std::make_error_code(std::errc::invalid_argument);
    return socket_error_retval;
  }

  if (level == custom_socket_option_level
      && optname == enable_connection_aborted_option)
  {
    if (*optlen != sizeof(int))
    {
      ec = std::make_error_code(std::errc::invalid_argument);
      return socket_error_retval;
    }

    *static_cast<int*>(optval) = (state & enable_connection_aborted) ? 1 : 0;
    ec = std::error_code();
    return 0;
  }

#if defined(__BORLANDC__)
  // Mysteriously, using the getsockopt and setsockopt functions directly with
  // Borland C++ results in incorrect values being set and read. The bug can be
  // worked around by using function addresses resolved with GetProcAddress.
  if (HMODULE winsock_module = ::GetModuleHandleA("ws2_32"))
  {
    typedef int (WSAAPI *gso_t)(SOCKET, int, int, char*, int*);
    if (gso_t gso = (gso_t)::GetProcAddress(winsock_module, "getsockopt"))
    {
      clear_last_error();
      int tmp_optlen = static_cast<int>(*optlen);
      int result = error_wrapper(gso(s, level, optname,
            reinterpret_cast<char*>(optval), &tmp_optlen), ec);
      *optlen = static_cast<size_t>(tmp_optlen);
      if (result != 0 && level == IPPROTO_IPV6 && optname == IPV6_V6ONLY
          && ec.value() == WSAENOPROTOOPT && *optlen == sizeof(DWORD))
      {
        // Dual-stack IPv4/v6 sockets, and the IPV6_V6ONLY socket option, are
        // only supported on Windows Vista and later. To simplify program logic
        // we will fake success of getting this option and specify that the
        // value is non-zero (i.e. true). This corresponds to the behavior of
        // IPv6 sockets on Windows platforms pre-Vista.
        *static_cast<DWORD*>(optval) = 1;
        ec = std::error_code();
      }
      return result;
    }
  }
  ec = std::errc::bad_address;
  return socket_error_retval;
#elif defined(_WIN32) || defined(__CYGWIN__)
  clear_last_error();
  int result = error_wrapper(call_getsockopt(&msghdr::msg_namelen,
        s, level, optname, optval, optlen), ec);
  if (result != 0 && level == IPPROTO_IPV6 && optname == IPV6_V6ONLY
      && ec.value() == WSAENOPROTOOPT && *optlen == sizeof(DWORD))
  {
    // Dual-stack IPv4/v6 sockets, and the IPV6_V6ONLY socket option, are only
    // supported on Windows Vista and later. To simplify program logic we will
    // fake success of getting this option and specify that the value is
    // non-zero (i.e. true). This corresponds to the behavior of IPv6 sockets
    // on Windows platforms pre-Vista.
    *static_cast<DWORD*>(optval) = 1;
    ec = std::error_code();
  }
  if (result == 0)
    ec = std::error_code();
  return result;
#else // defined(_WIN32) || defined(__CYGWIN__)
  clear_last_error();
  int result = error_wrapper(call_getsockopt(&msghdr::msg_namelen,
        s, level, optname, optval, optlen), ec);
#if defined(__linux__)
  if (result == 0 && level == SOL_SOCKET && *optlen == sizeof(int)
      && (optname == SO_SNDBUF || optname == SO_RCVBUF))
  {
    // On Linux, setting SO_SNDBUF or SO_RCVBUF to N actually causes the kernel
    // to set the buffer size to N*2. Linux puts additional stuff into the
    // buffers so that only about half is actually available to the application.
    // The retrieved value is divided by 2 here to make it appear as though the
    // correct value has been set.
    *static_cast<int*>(optval) /= 2;
  }
#endif // defined(__linux__)
  if (result == 0)
    ec = std::error_code();
  return result;
#endif // defined(_WIN32) || defined(__CYGWIN__)
}

template <typename SockLenType>
inline int call_getpeername(SockLenType msghdr::*,
    socket_type s, socket_addr_type* addr, std::size_t* addrlen)
{
  SockLenType tmp_addrlen = (SockLenType)*addrlen;
  int result = ::getpeername(s, addr, &tmp_addrlen);
  *addrlen = (std::size_t)tmp_addrlen;
  return result;
}

int getpeername(socket_type s, socket_addr_type* addr,
    std::size_t* addrlen, bool cached, std::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = std::make_error_code(std::errc::bad_file_descriptor);
    return socket_error_retval;
  }

#if defined(_WIN32) || defined(__CYGWIN__)
  if (cached)
  {
    // Check if socket is still connected.
    DWORD connect_time = 0;
    size_t connect_time_len = sizeof(connect_time);
    if (socket_ops::getsockopt(s, 0, SOL_SOCKET, SO_CONNECT_TIME,
          &connect_time, &connect_time_len, ec) == socket_error_retval)
    {
      return socket_error_retval;
    }
    if (connect_time == 0xFFFFFFFF)
    {
      ec = std::make_error_code(std::errc::not_connected);
      return socket_error_retval;
    }

    // The cached value is still valid.
    ec = std::error_code();
    return 0;
  }
#else // defined(_WIN32) || defined(__CYGWIN__)
  (void)cached;
#endif // defined(_WIN32) || defined(__CYGWIN__)

  clear_last_error();
  int result = error_wrapper(call_getpeername(
        &msghdr::msg_namelen, s, addr, addrlen), ec);
  if (result == 0)
    ec = std::error_code();
  return result;
}

template <typename SockLenType>
inline int call_getsockname(SockLenType msghdr::*,
    socket_type s, socket_addr_type* addr, std::size_t* addrlen)
{
  SockLenType tmp_addrlen = (SockLenType)*addrlen;
  int result = ::getsockname(s, addr, &tmp_addrlen);
  *addrlen = (std::size_t)tmp_addrlen;
  return result;
}

int getsockname(socket_type s, socket_addr_type* addr,
    std::size_t* addrlen, std::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = std::make_error_code(std::errc::bad_file_descriptor);
    return socket_error_retval;
  }

  clear_last_error();
  int result = error_wrapper(call_getsockname(
        &msghdr::msg_namelen, s, addr, addrlen), ec);
  if (result == 0)
    ec = std::error_code();
  return result;
}

int ioctl(socket_type s, state_type& state, int cmd,
    ioctl_arg_type* arg, std::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = std::make_error_code(std::errc::bad_file_descriptor);
    return socket_error_retval;
  }

  clear_last_error();
#if defined(_WIN32) || defined(__CYGWIN__)
  int result = error_wrapper(::ioctlsocket(s, cmd, arg), ec);
#elif defined(__MACH__) && defined(__APPLE__) \
  || defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
  int result = error_wrapper(::ioctl(s,
        static_cast<unsigned int>(cmd), arg), ec);
#else
  int result = error_wrapper(::ioctl(s, cmd, arg), ec);
#endif
  if (result >= 0)
  {
    ec = std::error_code();

    // When updating the non-blocking mode we always perform the ioctl syscall,
    // even if the flags would otherwise indicate that the socket is already in
    // the correct state. This ensures that the underlying socket is put into
    // the state that has been requested by the user. If the ioctl syscall was
    // successful then we need to update the flags to match.
    if (cmd == static_cast<int>(FIONBIO))
    {
      if (*arg)
      {
        state |= user_set_non_blocking;
      }
      else
      {
        // Clearing the non-blocking mode always overrides any internally-set
        // non-blocking flag. Any subsequent asynchronous operations will need
        // to re-enable non-blocking I/O.
        state &= ~(user_set_non_blocking | internal_non_blocking);
      }
    }
  }

  return result;
}

int select(int nfds, fd_set* readfds, fd_set* writefds,
    fd_set* exceptfds, timeval* timeout, std::error_code& ec)
{
  clear_last_error();
#if defined(_WIN32) || defined(__CYGWIN__)
  if (!readfds && !writefds && !exceptfds && timeout)
  {
    DWORD milliseconds = timeout->tv_sec * 1000 + timeout->tv_usec / 1000;
    if (milliseconds == 0)
      milliseconds = 1; // Force context switch.
    ::Sleep(milliseconds);
    ec = std::error_code();
    return 0;
  }

  // The select() call allows timeout values measured in microseconds, but the
  // system clock (as wrapped by boost::posix_time::microsec_clock) typically
  // has a resolution of 10 milliseconds. This can lead to a spinning select
  // reactor, meaning increased CPU usage, when waiting for the earliest
  // scheduled timeout if it's less than 10 milliseconds away. To avoid a tight
  // spin we'll use a minimum timeout of 1 millisecond.
  if (timeout && timeout->tv_sec == 0
      && timeout->tv_usec > 0 && timeout->tv_usec < 1000)
    timeout->tv_usec = 1000;
#endif // defined(_WIN32) || defined(__CYGWIN__)

#if defined(__hpux) && defined(__SELECT)
  timespec ts;
  ts.tv_sec = timeout ? timeout->tv_sec : 0;
  ts.tv_nsec = timeout ? timeout->tv_usec * 1000 : 0;
  return error_wrapper(::pselect(nfds, readfds,
        writefds, exceptfds, timeout ? &ts : 0, 0), ec);
#else
  int result = error_wrapper(::select(nfds, readfds,
        writefds, exceptfds, timeout), ec);
  if (result >= 0)
    ec = std::error_code();
  return result;
#endif
}

int poll_read(socket_type s, state_type state,
    int msec, std::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = std::make_error_code(std::errc::bad_file_descriptor);
    return socket_error_retval;
  }

#if defined(_WIN32) \
  || defined(__CYGWIN__) \
  || defined(__SYMBIAN32__)
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(s, &fds);
  timeval timeout_obj;
  timeval* timeout;
  if (state & user_set_non_blocking)
  {
    timeout_obj.tv_sec = 0;
    timeout_obj.tv_usec = 0;
    timeout = &timeout_obj;
  }
  else if (msec >= 0)
  {
    timeout_obj.tv_sec = msec / 1000;
    timeout_obj.tv_usec = (msec % 1000) * 1000;
    timeout = &timeout_obj;
  }
  else
    timeout = 0;
  clear_last_error();
  int result = error_wrapper(::select(s + 1, &fds, 0, 0, timeout), ec);
#else // defined(_WIN32)
      // || defined(__CYGWIN__)
      // || defined(__SYMBIAN32__)
  pollfd fds;
  fds.fd = s;
  fds.events = POLLIN;
  fds.revents = 0;
  int timeout = (state & user_set_non_blocking) ? 0 : msec;
  clear_last_error();
  int result = error_wrapper(::poll(&fds, 1, timeout), ec);
#endif // defined(_WIN32)
       // || defined(__CYGWIN__)
       // || defined(__SYMBIAN32__)
  if (result == 0)
    ec = (state & user_set_non_blocking) ? std::make_error_code(std::errc::operation_would_block) : std::error_code();
  else if (result > 0)
    ec = std::error_code();
  return result;
}

int poll_write(socket_type s, state_type state,
    int msec, std::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = std::make_error_code(std::errc::bad_file_descriptor);
    return socket_error_retval;
  }

#if defined(_WIN32) \
  || defined(__CYGWIN__) \
  || defined(__SYMBIAN32__)
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(s, &fds);
  timeval timeout_obj;
  timeval* timeout;
  if (state & user_set_non_blocking)
  {
    timeout_obj.tv_sec = 0;
    timeout_obj.tv_usec = 0;
    timeout = &timeout_obj;
  }
  else if (msec >= 0)
  {
    timeout_obj.tv_sec = msec / 1000;
    timeout_obj.tv_usec = (msec % 1000) * 1000;
    timeout = &timeout_obj;
  }
  else
    timeout = 0;
  clear_last_error();
  int result = error_wrapper(::select(s + 1, 0, &fds, 0, timeout), ec);
#else // defined(_WIN32)
      // || defined(__CYGWIN__)
      // || defined(__SYMBIAN32__)
  pollfd fds;
  fds.fd = s;
  fds.events = POLLOUT;
  fds.revents = 0;
  int timeout = (state & user_set_non_blocking) ? 0 : msec;
  clear_last_error();
  int result = error_wrapper(::poll(&fds, 1, timeout), ec);
#endif // defined(_WIN32)
       // || defined(__CYGWIN__)
       // || defined(__SYMBIAN32__)
  if (result == 0)
    ec = (state & user_set_non_blocking) ? std::make_error_code(std::errc::operation_would_block) : std::error_code();
  else if (result > 0)
    ec = std::error_code();
  return result;
}

int poll_error(socket_type s, state_type state,
    int msec, std::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = std::make_error_code(std::errc::bad_file_descriptor);
    return socket_error_retval;
  }

#if defined(_WIN32) \
  || defined(__CYGWIN__) \
  || defined(__SYMBIAN32__)
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(s, &fds);
  timeval timeout_obj;
  timeval* timeout;
  if (state & user_set_non_blocking)
  {
    timeout_obj.tv_sec = 0;
    timeout_obj.tv_usec = 0;
    timeout = &timeout_obj;
  }
  else if (msec >= 0)
  {
    timeout_obj.tv_sec = msec / 1000;
    timeout_obj.tv_usec = (msec % 1000) * 1000;
    timeout = &timeout_obj;
  }
  else
    timeout = 0;
  clear_last_error();
  int result = error_wrapper(::select(s + 1, 0, 0, &fds, timeout), ec);
#else // defined(_WIN32)
      // || defined(__CYGWIN__)
      // || defined(__SYMBIAN32__)
  pollfd fds;
  fds.fd = s;
  fds.events = POLLPRI | POLLERR | POLLHUP;
  fds.revents = 0;
  int timeout = (state & user_set_non_blocking) ? 0 : msec;
  clear_last_error();
  int result = error_wrapper(::poll(&fds, 1, timeout), ec);
#endif // defined(_WIN32)
       // || defined(__CYGWIN__)
       // || defined(__SYMBIAN32__)
  if (result == 0)
    ec = (state & user_set_non_blocking) ? std::make_error_code(std::errc::operation_would_block) : std::error_code();
  else if (result > 0)
    ec = std::error_code();
  return result;
}

int poll_connect(socket_type s, int msec, std::error_code& ec)
{
  if (s == invalid_socket)
  {
    ec = std::make_error_code(std::errc::bad_file_descriptor);
    return socket_error_retval;
  }

#if defined(_WIN32) \
  || defined(__CYGWIN__) \
  || defined(__SYMBIAN32__)
  fd_set write_fds;
  FD_ZERO(&write_fds);
  FD_SET(s, &write_fds);
  fd_set except_fds;
  FD_ZERO(&except_fds);
  FD_SET(s, &except_fds);
  timeval timeout_obj;
  timeval* timeout;
  if (msec >= 0)
  {
    timeout_obj.tv_sec = msec / 1000;
    timeout_obj.tv_usec = (msec % 1000) * 1000;
    timeout = &timeout_obj;
  }
  else
    timeout = 0;
  clear_last_error();
  int result = error_wrapper(::select(
        s + 1, 0, &write_fds, &except_fds, timeout), ec);
  if (result >= 0)
    ec = std::error_code();
  return result;
#else // defined(_WIN32)
      // || defined(__CYGWIN__)
      // || defined(__SYMBIAN32__)
  pollfd fds;
  fds.fd = s;
  fds.events = POLLOUT;
  fds.revents = 0;
  clear_last_error();
  int result = error_wrapper(::poll(&fds, 1, msec), ec);
  if (result >= 0)
    ec = std::error_code();
  return result;
#endif // defined(_WIN32)
       // || defined(__CYGWIN__)
       // || defined(__SYMBIAN32__)
}

const char* inet_ntop(int af, const void* src, char* dest, size_t length,
    unsigned long scope_id, std::error_code& ec)
{
  clear_last_error();
#if defined(_WIN32) || defined(__CYGWIN__)
  using namespace std; // For memcpy.

  if (af != NET_OS_DEF(AF_INET) && af != NET_OS_DEF(AF_INET6))
  {
    ec = std::make_error_code(std::errc::address_family_not_supported);
    return 0;
  }

  union
  {
    socket_addr_type base;
    sockaddr_storage_type storage;
    sockaddr_in4_type v4;
    sockaddr_in6_type v6;
  } address;
  DWORD address_length;
  if (af == NET_OS_DEF(AF_INET))
  {
    address_length = sizeof(sockaddr_in4_type);
    address.v4.sin_family = NET_OS_DEF(AF_INET);
    address.v4.sin_port = 0;
    memcpy(&address.v4.sin_addr, src, sizeof(in4_addr_type));
  }
  else // AF_INET6
  {
    address_length = sizeof(sockaddr_in6_type);
    address.v6.sin6_family = NET_OS_DEF(AF_INET6);
    address.v6.sin6_port = 0;
    address.v6.sin6_flowinfo = 0;
    address.v6.sin6_scope_id = scope_id;
    memcpy(&address.v6.sin6_addr, src, sizeof(in6_addr_type));
  }

  DWORD string_length = static_cast<DWORD>(length);
#if (defined(_UNICODE) || defined(UNICODE)) && (defined(_MSC_VER) && _MSC_VER >= 1800)
  LPWSTR string_buffer = (LPWSTR)_alloca(length * sizeof(WCHAR));
  int result = error_wrapper(::WSAAddressToStringW(&address.base,
        address_length, 0, string_buffer, &string_length), ec);
  ::WideCharToMultiByte(CP_ACP, 0, string_buffer, -1,
      dest, static_cast<int>(length), 0, 0);
#else
  int result = error_wrapper(::WSAAddressToStringA(
        &address.base, address_length, 0, dest, &string_length), ec);
#endif

  // Windows may set error code on success.
  if (result != socket_error_retval)
    ec = std::error_code();

  // Windows may not set an error code on failure.
  else if (result == socket_error_retval && !ec)
    ec = std::make_error_code(std::errc::invalid_argument);

  return result == socket_error_retval ? 0 : dest;
#else // defined(_WIN32) || defined(__CYGWIN__)
  const char* result = error_wrapper(::inet_ntop(
        af, src, dest, static_cast<int>(length)), ec);
  if (result == 0 && !ec)
    ec = std::make_error_code(std::errc::invalid_argument);
  if (result != 0 && af == NET_OS_DEF(AF_INET6) && scope_id != 0)
  {
    using namespace std; // For strcat and sprintf.
    char if_name[IF_NAMESIZE + 1] = "%";
    const in6_addr_type* ipv6_address = static_cast<const in6_addr_type*>(src);
    bool is_link_local = ((ipv6_address->s6_addr[0] == 0xfe)
        && ((ipv6_address->s6_addr[1] & 0xc0) == 0x80));
    bool is_multicast_link_local = ((ipv6_address->s6_addr[0] == 0xff)
        && ((ipv6_address->s6_addr[1] & 0x0f) == 0x02));
    if ((!is_link_local && !is_multicast_link_local)
        || if_indextoname(static_cast<unsigned>(scope_id), if_name + 1) == 0)
      sprintf(if_name + 1, "%lu", scope_id);
    strcat(dest, if_name);
  }
  return result;
#endif // defined(_WIN32) || defined(__CYGWIN__)
}

int inet_pton(int af, const char* src, void* dest,
    unsigned long* scope_id, std::error_code& ec)
{
  clear_last_error();
#if defined(_WIN32) || defined(__CYGWIN__)
  using namespace std; // For memcpy and strcmp.

  if (af != NET_OS_DEF(AF_INET) && af != NET_OS_DEF(AF_INET6))
  {
    ec = std::make_error_code(std::errc::address_family_not_supported);
    return -1;
  }

  union
  {
    socket_addr_type base;
    sockaddr_storage_type storage;
    sockaddr_in4_type v4;
    sockaddr_in6_type v6;
  } address;
  int address_length = sizeof(sockaddr_storage_type);
#if defined(_UNICODE) || defined(UNICODE) || (defined(_MSC_VER) && (_MSC_VER >= 1800))
  int num_wide_chars = static_cast<int>(strlen(src)) + 1;
  LPWSTR wide_buffer = (LPWSTR)_alloca(num_wide_chars * sizeof(WCHAR));
  ::MultiByteToWideChar(CP_ACP, 0, src, -1, wide_buffer, num_wide_chars);
  int result = error_wrapper(::WSAStringToAddressW(
        wide_buffer, af, 0, &address.base, &address_length), ec);
#else
  int result = error_wrapper(::WSAStringToAddressA(
        const_cast<char*>(src), af, 0, &address.base, &address_length), ec);
#endif

  if (af == NET_OS_DEF(AF_INET))
  {
    if (result != socket_error_retval)
    {
      memcpy(dest, &address.v4.sin_addr, sizeof(in4_addr_type));
      ec = std::error_code();
    }
    else if (strcmp(src, "255.255.255.255") == 0)
    {
      static_cast<in4_addr_type*>(dest)->s_addr = INADDR_NONE;
      ec = std::error_code();
    }
  }
  else // AF_INET6
  {
    if (result != socket_error_retval)
    {
      memcpy(dest, &address.v6.sin6_addr, sizeof(in6_addr_type));
      if (scope_id)
        *scope_id = address.v6.sin6_scope_id;
      ec = std::error_code();
    }
  }

  // Windows may not set an error code on failure.
  if (result == socket_error_retval && !ec)
    ec = std::make_error_code(std::errc::invalid_argument);

  if (result != socket_error_retval)
    ec = std::error_code();

  return result == socket_error_retval ? -1 : 1;
#else // defined(_WIN32) || defined(__CYGWIN__)
  using namespace std; // For strchr, memcpy and atoi.

  // On some platforms, inet_pton fails if an address string contains a scope
  // id. Detect and remove the scope id before passing the string to inet_pton.
  const bool is_v6 = (af == NET_OS_DEF(AF_INET6));
  const char* if_name = is_v6 ? strchr(src, '%') : 0;
  char src_buf[max_addr_v6_str_len + 1];
  const char* src_ptr = src;
  if (if_name != 0)
  {
    if (if_name - src > max_addr_v6_str_len)
    {
      ec = std::make_error_code(std::errc::invalid_argument);
      return 0;
    }
    memcpy(src_buf, src, if_name - src);
    src_buf[if_name - src] = 0;
    src_ptr = src_buf;
  }

  int result = error_wrapper(::inet_pton(af, src_ptr, dest), ec);
  if (result <= 0 && !ec)
    ec = std::make_error_code(std::errc::invalid_argument);
  if (result > 0 && is_v6 && scope_id)
  {
    using namespace std; // For strchr and atoi.
    *scope_id = 0;
    if (if_name != 0)
    {
      in6_addr_type* ipv6_address = static_cast<in6_addr_type*>(dest);
      bool is_link_local = ((ipv6_address->s6_addr[0] == 0xfe)
          && ((ipv6_address->s6_addr[1] & 0xc0) == 0x80));
      bool is_multicast_link_local = ((ipv6_address->s6_addr[0] == 0xff)
          && ((ipv6_address->s6_addr[1] & 0x0f) == 0x02));
      if (is_link_local || is_multicast_link_local)
        *scope_id = if_nametoindex(if_name + 1);
      if (*scope_id == 0)
        *scope_id = atoi(if_name + 1);
    }
  }
  return result;
#endif // defined(_WIN32) || defined(__CYGWIN__)
}

int gethostname(char* name, int namelen, std::error_code& ec)
{
  clear_last_error();
  int result = error_wrapper(::gethostname(name, namelen), ec);
# if defined(_WIN32)
  if (result == 0)
    ec = std::error_code();
# endif // defined(_WIN32)
  return result;
}

inline std::error_code translate_addrinfo_error(int error)
{
  switch (error)
  {
  case 0:
    return std::error_code();
  case EAI_AGAIN:
    return std::error_code(NET_NETDB_ERROR(TRY_AGAIN), std::system_category());
  case EAI_BADFLAGS:
    return std::make_error_code(std::errc::invalid_argument);
  case EAI_FAIL:
    return std::error_code(NET_NETDB_ERROR(NO_RECOVERY), std::system_category());
  case EAI_FAMILY:
    return std::make_error_code(std::errc::address_family_not_supported);
  case EAI_MEMORY:
    return std::make_error_code(std::errc::not_enough_memory);
  case EAI_NONAME:
#if defined(EAI_ADDRFAMILY)
  case EAI_ADDRFAMILY:
#endif
#if defined(EAI_NODATA) && (EAI_NODATA != EAI_NONAME)
  case EAI_NODATA:
#endif
    return std::error_code(NET_NETDB_ERROR(HOST_NOT_FOUND), std::system_category());
  case EAI_SERVICE:
      return std::error_code(NET_WIN_OR_POSIX(NET_NATIVE_ERROR(WSATYPE_NOT_FOUND), NET_GETADDRINFO_ERROR(EAI_SERVICE)), std::system_category());
  case EAI_SOCKTYPE:
      return std::error_code(NET_WIN_OR_POSIX(NET_NATIVE_ERROR(WSAESOCKTNOSUPPORT), NET_GETADDRINFO_ERROR(EAI_SOCKTYPE)), std::system_category());
  default: // Possibly the non-portable EAI_SYSTEM.
      return std::error_code(get_error_code(), std::generic_category());
  }
}

std::error_code getaddrinfo(const char* host,
    const char* service, const addrinfo_type& hints,
    addrinfo_type** result, std::error_code& ec)
{
  host = (host && *host) ? host : 0;
  service = (service && *service) ? service : 0;
  clear_last_error();
#if defined(_WIN32) || defined(__CYGWIN__)

  // Building for Windows XP, Windows Server 2003, or later.
  int error = ::getaddrinfo(host, service, &hints, result);
  return ec = translate_addrinfo_error(error);
#else
  int error = ::getaddrinfo(host, service, &hints, result);
#if defined(__MACH__) && defined(__APPLE__)
  using namespace std; // For isdigit and atoi.
  if (error == 0 && service && isdigit(static_cast<unsigned char>(service[0])))
  {
    u_short_type port = host_to_network_short(atoi(service));
    for (addrinfo_type* ai = *result; ai; ai = ai->ai_next)
    {
      switch (ai->ai_family)
      {
      case NET_OS_DEF(AF_INET):
        {
          sockaddr_in4_type* sinptr =
            reinterpret_cast<sockaddr_in4_type*>(ai->ai_addr);
          if (sinptr->sin_port == 0)
            sinptr->sin_port = port;
          break;
        }
      case NET_OS_DEF(AF_INET6):
        {
          sockaddr_in6_type* sin6ptr =
            reinterpret_cast<sockaddr_in6_type*>(ai->ai_addr);
          if (sin6ptr->sin6_port == 0)
            sin6ptr->sin6_port = port;
          break;
        }
      default:
        break;
      }
    }
  }
#endif //#if defined(__MACH__) && defined(__APPLE__)
  return ec = translate_addrinfo_error(error);
#endif //#if defined(_WIN32) || defined(__CYGWIN__)
}

std::error_code background_getaddrinfo(
    const weak_cancel_token_type& cancel_token, const char* host,
    const char* service, const addrinfo_type& hints,
    addrinfo_type** result, std::error_code& ec)
{
  if (cancel_token.expired())
    ec = std::make_error_code(std::errc::operation_canceled);
  else
    socket_ops::getaddrinfo(host, service, hints, result, ec);
  return ec;
}

void freeaddrinfo(addrinfo_type* ai)
{
    ::freeaddrinfo(ai);
}

std::error_code getnameinfo(const socket_addr_type* addr,
    std::size_t addrlen, char* host, std::size_t hostlen,
    char* serv, std::size_t servlen, int flags, std::error_code& ec)
{
    clear_last_error();
#if defined(_WIN32) || defined(__CYGWIN__)
  // Building for Windows XP, Windows Server 2003, or later.
  int error = ::getnameinfo(addr, static_cast<socklen_t>(addrlen),
      host, static_cast<DWORD>(hostlen),
      serv, static_cast<DWORD>(servlen), flags);
#else
  int error = ::getnameinfo(addr, (socklen_t)addrlen, host, (socklen_t)hostlen, serv, (socklen_t)servlen, flags);
#endif
  return ec = translate_addrinfo_error(error);
}

std::error_code sync_getnameinfo(
    const socket_addr_type* addr, std::size_t addrlen,
    char* host, std::size_t hostlen, char* serv,
    std::size_t servlen, int sock_type, std::error_code& ec)
{
  // First try resolving with the service name. If that fails try resolving
  // but allow the service to be returned as a number.
  int flags = (sock_type == SOCK_DGRAM) ? NI_DGRAM : 0;
  socket_ops::getnameinfo(addr, addrlen, host,
      hostlen, serv, servlen, flags, ec);
  if (ec)
  {
    socket_ops::getnameinfo(addr, addrlen, host, hostlen,
        serv, servlen, flags | NI_NUMERICSERV, ec);
  }

  return ec;
}

std::error_code background_getnameinfo(
    const weak_cancel_token_type& cancel_token,
    const socket_addr_type* addr, std::size_t addrlen,
    char* host, std::size_t hostlen, char* serv,
    std::size_t servlen, int sock_type, std::error_code& ec)
{
  if (cancel_token.expired())
  {
    ec = std::make_error_code(std::errc::operation_canceled);
  }
  else
  {
    // First try resolving with the service name. If that fails try resolving
    // but allow the service to be returned as a number.
    int flags = (sock_type == SOCK_DGRAM) ? NI_DGRAM : 0;
    socket_ops::getnameinfo(addr, addrlen, host,
        hostlen, serv, servlen, flags, ec);
    if (ec)
    {
      socket_ops::getnameinfo(addr, addrlen, host, hostlen,
          serv, servlen, flags | NI_NUMERICSERV, ec);
    }
  }
  return ec;
}


u_long_type network_to_host_long(u_long_type value)
{
#if PLATFORM_WINRT
  unsigned char* value_p = reinterpret_cast<unsigned char*>(&value);
  u_long_type result = (static_cast<u_long_type>(value_p[0]) << 24)
    | (static_cast<u_long_type>(value_p[1]) << 16)
    | (static_cast<u_long_type>(value_p[2]) << 8)
    | static_cast<u_long_type>(value_p[3]);
  return result;
#else // PLATFORM_WINRT
  return ntohl(value);
#endif // PLATFORM_WINRT
}

u_long_type host_to_network_long(u_long_type value)
{
#if PLATFORM_WINRT
  u_long_type result;
  unsigned char* result_p = reinterpret_cast<unsigned char*>(&result);
  result_p[0] = static_cast<unsigned char>((value >> 24) & 0xFF);
  result_p[1] = static_cast<unsigned char>((value >> 16) & 0xFF);
  result_p[2] = static_cast<unsigned char>((value >> 8) & 0xFF);
  result_p[3] = static_cast<unsigned char>(value & 0xFF);
  return result;
#else // PLATFORM_WINRT
  return htonl(value);
#endif // PLATFORM_WINRT
}

u_short_type network_to_host_short(u_short_type value)
{
#if PLATFORM_WINRT
  unsigned char* value_p = reinterpret_cast<unsigned char*>(&value);
  u_short_type result = (static_cast<u_short_type>(value_p[0]) << 8)
    | static_cast<u_short_type>(value_p[1]);
  return result;
#else // PLATFORM_WINRT
  return ntohs(value);
#endif // PLATFORM_WINRT
}

u_short_type host_to_network_short(u_short_type value)
{
#if PLATFORM_WINRT
  u_short_type result;
  unsigned char* result_p = reinterpret_cast<unsigned char*>(&result);
  result_p[0] = static_cast<unsigned char>((value >> 8) & 0xFF);
  result_p[1] = static_cast<unsigned char>(value & 0xFF);
  return result;
#else // PLATFORM_WINRT
  return htons(value);
#endif // PLATFORM_WINRT
}

} // namespace socket_ops
} // namespace NetLite

#endif // END OF NETLITE_SOCKET_OPS_IPP
