
#ifndef NETLITE_SOCKET_TYPES_HPP
#define NETLITE_SOCKET_TYPES_HPP


#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)


#if defined(_WIN32)
# if defined(_WINSOCKAPI_) && !defined(_WINSOCK2API_)
#  error WinSock.h has already been included
# endif // defined(_WINSOCKAPI_) && !defined(_WINSOCK2API_)
# if defined(__BORLANDC__)
#  include <stdlib.h> // Needed for __errno
# endif // defined(__BORLANDC__)
# if defined(WINAPI_FAMILY)
#  if ((WINAPI_FAMILY & WINAPI_PARTITION_DESKTOP) != 0)
#   include <windows.h>
#  endif // ((WINAPI_FAMILY & WINAPI_PARTITION_DESKTOP) != 0)
# endif // defined(WINAPI_FAMILY)
# include <winsock2.h>
# include <ws2tcpip.h>
# include <Mswsock.h>

#  if defined(UNDER_CE)
#   pragma comment(lib, "ws2.lib")
#  elif defined(_MSC_VER) || defined(__BORLANDC__)
#   pragma comment(lib, "ws2_32.lib")
#  endif // defined(_MSC_VER) || defined(__BORLANDC__)

#else // defined(_WIN32)

# include <sys/ioctl.h>
# if !defined(__SYMBIAN32__)
#  include <sys/poll.h>
# endif
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# if defined(__hpux)
#  include <sys/time.h>
# endif
# if !defined(__hpux) || defined(__SELECT)
#  include <sys/select.h>
# endif
# include <sys/socket.h>
# include <sys/uio.h>
# include <sys/un.h>
# include <netinet/in.h>
# if !defined(__SYMBIAN32__)
#  include <netinet/tcp.h>
# endif
# include <arpa/inet.h>
# include <netdb.h>
# include <net/if.h>
# include <limits.h>
# if defined(__sun)
#  include <sys/filio.h>
#  include <sys/sockio.h>
# endif
# include <unistd.h>
#endif



namespace NetLite{

#if defined(_WIN32)
    typedef SOCKET socket_type;
    const SOCKET invalid_socket = INVALID_SOCKET;
    const int socket_error_retval = SOCKET_ERROR;
    const int max_addr_v4_str_len = 256;
    const int max_addr_v6_str_len = 256;
    typedef sockaddr socket_addr_type;
    typedef in_addr in4_addr_type;
    typedef ip_mreq in4_mreq_type;
    typedef sockaddr_in sockaddr_in4_type;
    typedef in6_addr in6_addr_type;
    typedef ipv6_mreq in6_mreq_type;
    typedef sockaddr_in6 sockaddr_in6_type;
    typedef sockaddr_storage sockaddr_storage_type;
    typedef addrinfo addrinfo_type;
    typedef ::linger linger_type;
    typedef unsigned long ioctl_arg_type;
    typedef u_long u_long_type;
    typedef u_short u_short_type;
    typedef int signed_size_type;
# define NET_OS_DEF(c) NET_OS_DEF_##c
# define NET_OS_DEF_AF_UNSPEC AF_UNSPEC
# define NET_OS_DEF_AF_INET AF_INET
# define NET_OS_DEF_AF_INET6 AF_INET6
# define NET_OS_DEF_SOCK_STREAM SOCK_STREAM
# define NET_OS_DEF_SOCK_DGRAM SOCK_DGRAM
# define NET_OS_DEF_SOCK_RAW SOCK_RAW
# define NET_OS_DEF_SOCK_SEQPACKET SOCK_SEQPACKET
# define NET_OS_DEF_IPPROTO_IP IPPROTO_IP
# define NET_OS_DEF_IPPROTO_IPV6 IPPROTO_IPV6
# define NET_OS_DEF_IPPROTO_TCP IPPROTO_TCP
# define NET_OS_DEF_IPPROTO_UDP IPPROTO_UDP
# define NET_OS_DEF_IPPROTO_ICMP IPPROTO_ICMP
# define NET_OS_DEF_IPPROTO_ICMPV6 IPPROTO_ICMPV6
# define NET_OS_DEF_FIONBIO FIONBIO
# define NET_OS_DEF_FIONREAD FIONREAD
# define NET_OS_DEF_INADDR_ANY INADDR_ANY
# define NET_OS_DEF_MSG_OOB MSG_OOB
# define NET_OS_DEF_MSG_PEEK MSG_PEEK
# define NET_OS_DEF_MSG_DONTROUTE MSG_DONTROUTE
# define NET_OS_DEF_MSG_EOR 0 // Not supported on Windows.
# define NET_OS_DEF_SHUT_RD SD_RECEIVE
# define NET_OS_DEF_SHUT_WR SD_SEND
# define NET_OS_DEF_SHUT_RDWR SD_BOTH
# define NET_OS_DEF_SOMAXCONN SOMAXCONN
# define NET_OS_DEF_SOL_SOCKET SOL_SOCKET
# define NET_OS_DEF_SO_BROADCAST SO_BROADCAST
# define NET_OS_DEF_SO_DEBUG SO_DEBUG
# define NET_OS_DEF_SO_DONTROUTE SO_DONTROUTE
# define NET_OS_DEF_SO_KEEPALIVE SO_KEEPALIVE
# define NET_OS_DEF_SO_LINGER SO_LINGER
# define NET_OS_DEF_SO_SNDBUF SO_SNDBUF
# define NET_OS_DEF_SO_RCVBUF SO_RCVBUF
# define NET_OS_DEF_SO_SNDLOWAT SO_SNDLOWAT
# define NET_OS_DEF_SO_RCVLOWAT SO_RCVLOWAT
# define NET_OS_DEF_SO_REUSEADDR SO_REUSEADDR
# define NET_OS_DEF_TCP_NODELAY TCP_NODELAY
# define NET_OS_DEF_IP_MULTICAST_IF IP_MULTICAST_IF
# define NET_OS_DEF_IP_MULTICAST_TTL IP_MULTICAST_TTL
# define NET_OS_DEF_IP_MULTICAST_LOOP IP_MULTICAST_LOOP
# define NET_OS_DEF_IP_ADD_MEMBERSHIP IP_ADD_MEMBERSHIP
# define NET_OS_DEF_IP_DROP_MEMBERSHIP IP_DROP_MEMBERSHIP
# define NET_OS_DEF_IP_TTL IP_TTL
# define NET_OS_DEF_IPV6_UNICAST_HOPS IPV6_UNICAST_HOPS
# define NET_OS_DEF_IPV6_MULTICAST_IF IPV6_MULTICAST_IF
# define NET_OS_DEF_IPV6_MULTICAST_HOPS IPV6_MULTICAST_HOPS
# define NET_OS_DEF_IPV6_MULTICAST_LOOP IPV6_MULTICAST_LOOP
# define NET_OS_DEF_IPV6_JOIN_GROUP IPV6_JOIN_GROUP
# define NET_OS_DEF_IPV6_LEAVE_GROUP IPV6_LEAVE_GROUP
# define NET_OS_DEF_AI_CANONNAME AI_CANONNAME
# define NET_OS_DEF_AI_PASSIVE AI_PASSIVE
# define NET_OS_DEF_AI_NUMERICHOST AI_NUMERICHOST
# if defined(AI_NUMERICSERV)
#  define NET_OS_DEF_AI_NUMERICSERV AI_NUMERICSERV
# else
#  define NET_OS_DEF_AI_NUMERICSERV 0
# endif
# if defined(AI_V4MAPPED)
#  define NET_OS_DEF_AI_V4MAPPED AI_V4MAPPED
# else
#  define NET_OS_DEF_AI_V4MAPPED 0
# endif
# if defined(AI_ALL)
#  define NET_OS_DEF_AI_ALL AI_ALL
# else
#  define NET_OS_DEF_AI_ALL 0
# endif
# if defined(AI_ADDRCONFIG)
#  define NET_OS_DEF_AI_ADDRCONFIG AI_ADDRCONFIG
# else
#  define NET_OS_DEF_AI_ADDRCONFIG 0
# endif
# if defined (_WIN32_WINNT)
    const int max_iov_len = 64;
# else
    const int max_iov_len = 16;
# endif

#else // defined(_WIN32)

    typedef int socket_type;
    const int invalid_socket = -1;
    const int socket_error_retval = -1;
    const int max_addr_v4_str_len = INET_ADDRSTRLEN;
#if defined(INET6_ADDRSTRLEN)
    const int max_addr_v6_str_len = INET6_ADDRSTRLEN + 1 + IF_NAMESIZE;
#else // defined(INET6_ADDRSTRLEN)
    const int max_addr_v6_str_len = 256;
#endif // defined(INET6_ADDRSTRLEN)
    typedef sockaddr socket_addr_type;
    typedef in_addr in4_addr_type;
# if defined(__hpux)
    // HP-UX doesn't provide ip_mreq when _XOPEN_SOURCE_EXTENDED is defined.
    struct in4_mreq_type
    {
        struct in_addr imr_multiaddr;
        struct in_addr imr_interface;
    };
# else
    typedef ip_mreq in4_mreq_type;
# endif
    typedef sockaddr_in sockaddr_in4_type;
    typedef in6_addr in6_addr_type;
    typedef ipv6_mreq in6_mreq_type;
    typedef sockaddr_in6 sockaddr_in6_type;
    typedef sockaddr_storage sockaddr_storage_type;
    typedef sockaddr_un sockaddr_un_type;
    typedef addrinfo addrinfo_type;
    typedef ::linger linger_type;
    typedef int ioctl_arg_type;
    typedef uint32_t u_long_type;
    typedef uint16_t u_short_type;
# if defined(__linux__) || (defined(__MACH__) && defined(__APPLE__))
    typedef ssize_t signed_size_type;
#else // defined(__linux__) || (defined(__MACH__) && defined(__APPLE__))
    typedef int signed_size_type;
#endif // defined(__linux__) || (defined(__MACH__) && defined(__APPLE__))

# define NET_OS_DEF(c) NET_OS_DEF_##c
# define NET_OS_DEF_AF_UNSPEC AF_UNSPEC
# define NET_OS_DEF_AF_INET AF_INET
# define NET_OS_DEF_AF_INET6 AF_INET6
# define NET_OS_DEF_SOCK_STREAM SOCK_STREAM
# define NET_OS_DEF_SOCK_DGRAM SOCK_DGRAM
# define NET_OS_DEF_SOCK_RAW SOCK_RAW
# define NET_OS_DEF_SOCK_SEQPACKET SOCK_SEQPACKET
# define NET_OS_DEF_IPPROTO_IP IPPROTO_IP
# define NET_OS_DEF_IPPROTO_IPV6 IPPROTO_IPV6
# define NET_OS_DEF_IPPROTO_TCP IPPROTO_TCP
# define NET_OS_DEF_IPPROTO_UDP IPPROTO_UDP
# define NET_OS_DEF_IPPROTO_ICMP IPPROTO_ICMP
# define NET_OS_DEF_IPPROTO_ICMPV6 IPPROTO_ICMPV6
# define NET_OS_DEF_FIONBIO FIONBIO
# define NET_OS_DEF_FIONREAD FIONREAD
# define NET_OS_DEF_INADDR_ANY INADDR_ANY
# define NET_OS_DEF_MSG_OOB MSG_OOB
# define NET_OS_DEF_MSG_PEEK MSG_PEEK
# define NET_OS_DEF_MSG_DONTROUTE MSG_DONTROUTE
# define NET_OS_DEF_MSG_EOR MSG_EOR
# define NET_OS_DEF_SHUT_RD SHUT_RD
# define NET_OS_DEF_SHUT_WR SHUT_WR
# define NET_OS_DEF_SHUT_RDWR SHUT_RDWR
# define NET_OS_DEF_SOMAXCONN SOMAXCONN
# define NET_OS_DEF_SOL_SOCKET SOL_SOCKET
# define NET_OS_DEF_SO_BROADCAST SO_BROADCAST
# define NET_OS_DEF_SO_DEBUG SO_DEBUG
# define NET_OS_DEF_SO_DONTROUTE SO_DONTROUTE
# define NET_OS_DEF_SO_KEEPALIVE SO_KEEPALIVE
# define NET_OS_DEF_SO_LINGER SO_LINGER
# define NET_OS_DEF_SO_SNDBUF SO_SNDBUF
# define NET_OS_DEF_SO_RCVBUF SO_RCVBUF
# define NET_OS_DEF_SO_SNDLOWAT SO_SNDLOWAT
# define NET_OS_DEF_SO_RCVLOWAT SO_RCVLOWAT
# define NET_OS_DEF_SO_REUSEADDR SO_REUSEADDR
# define NET_OS_DEF_TCP_NODELAY TCP_NODELAY
# define NET_OS_DEF_IP_MULTICAST_IF IP_MULTICAST_IF
# define NET_OS_DEF_IP_MULTICAST_TTL IP_MULTICAST_TTL
# define NET_OS_DEF_IP_MULTICAST_LOOP IP_MULTICAST_LOOP
# define NET_OS_DEF_IP_ADD_MEMBERSHIP IP_ADD_MEMBERSHIP
# define NET_OS_DEF_IP_DROP_MEMBERSHIP IP_DROP_MEMBERSHIP
# define NET_OS_DEF_IP_TTL IP_TTL
# define NET_OS_DEF_IPV6_UNICAST_HOPS IPV6_UNICAST_HOPS
# define NET_OS_DEF_IPV6_MULTICAST_IF IPV6_MULTICAST_IF
# define NET_OS_DEF_IPV6_MULTICAST_HOPS IPV6_MULTICAST_HOPS
# define NET_OS_DEF_IPV6_MULTICAST_LOOP IPV6_MULTICAST_LOOP
# define NET_OS_DEF_IPV6_JOIN_GROUP IPV6_JOIN_GROUP
# define NET_OS_DEF_IPV6_LEAVE_GROUP IPV6_LEAVE_GROUP
# define NET_OS_DEF_AI_CANONNAME AI_CANONNAME
# define NET_OS_DEF_AI_PASSIVE AI_PASSIVE
# define NET_OS_DEF_AI_NUMERICHOST AI_NUMERICHOST
# if defined(AI_NUMERICSERV)
#  define NET_OS_DEF_AI_NUMERICSERV AI_NUMERICSERV
# else
#  define NET_OS_DEF_AI_NUMERICSERV 0
# endif
    // Note: QNX Neutrino 6.3 defines AI_V4MAPPED, AI_ALL and AI_ADDRCONFIG but
    // does not implement them. Therefore they are specifically excluded here.
# if defined(AI_V4MAPPED) && !defined(__QNXNTO__)
#  define NET_OS_DEF_AI_V4MAPPED AI_V4MAPPED
# else
#  define NET_OS_DEF_AI_V4MAPPED 0
# endif
# if defined(AI_ALL) && !defined(__QNXNTO__)
#  define NET_OS_DEF_AI_ALL AI_ALL
# else
#  define NET_OS_DEF_AI_ALL 0
# endif
# if defined(AI_ADDRCONFIG) && !defined(__QNXNTO__)
#  define NET_OS_DEF_AI_ADDRCONFIG AI_ADDRCONFIG
# else
#  define NET_OS_DEF_AI_ADDRCONFIG 0
# endif
# if defined(IOV_MAX)
    const int max_iov_len = IOV_MAX;
# else
    // POSIX platforms are not required to define IOV_MAX.
    const int max_iov_len = 16;
# endif
#endif
    const int custom_socket_option_level = 0xA5100000;
    const int enable_connection_aborted_option = 1;
    const int always_fail_option = 2;

} // namespace NetLite


#endif // END OF NETLITE_SOCKET_TYPES_HPP
