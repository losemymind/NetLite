/****************************************************************************
  Copyright (c) 2018 libo All rights reserved.
  
  losemymind.libo@gmail.com

****************************************************************************/
#ifndef NETLITE_CONFIG_HPP
#define NETLITE_CONFIG_HPP

/** 
 By default, NetLite is a header-only library.
 However, some developers may prefer to build NetLite using separately compiled source code.
 To do this, add #include <NetLite/detail/src.hpp> to one(and only one) source file in a program,
 then build the program with NETWORK_SEPARATE_COMPILATION defined in the project / compiler settings.
 Alternatively, NETWORK_DYNAMIC_LINK may be defined to build a separately - compiled NetLite as part of a shared library.

 Defined these macors in the project / compiler settings if need.
 |--------------------------------------------|--------------------------------------------------------------|
 | Macro                                      | Description                                                  |
 |--------------------------------------------|--------------------------------------------------------------|
 | NETWORK_SEPARATE_COMPILATION               | Build NetLite using separately compiled source code.      |
 |--------------------------------------------|--------------------------------------------------------------|
 | NETWORK_DYNAMIC_LINK                       | Build NetLite as part of a shared library.                |
 |--------------------------------------------|--------------------------------------------------------------|
 | NETWORK_DISABLE_EPOLL                      | Disable epoll if need.                                       |
 |--------------------------------------------|--------------------------------------------------------------|
 | NETWORK_DISABLE_EVENTFD                    | Disable eventfd if need.                                       |
 |--------------------------------------------|--------------------------------------------------------------|
 */


// Default to a header-only implementation. The user must specifically request
// separate compilation by defining either ASIO_SEPARATE_COMPILATION or
// NETWORK_DYNAMIC_LINK (as a DLL/shared library implies separate compilation).
#if !defined(NETWORK_HEADER_ONLY)
# if !defined(NETWORK_SEPARATE_COMPILATION)
#  if !defined(NETWORK_DYNAMIC_LINK)
#   define NETWORK_HEADER_ONLY 1
#  endif // !defined(NETWORK_DYNAMIC_LINK)
# endif // !defined(NETWORK_SEPARATE_COMPILATION)
#endif // !defined(NETWORK_HEADER_ONLY)


#if defined(NETWORK_HEADER_ONLY)
# define NETWORK_API inline
#else // defined(ASIO_HEADER_ONLY)
# if defined(_MSC_VER) || defined(__BORLANDC__) || defined(__CODEGEARC__)
// We need to import/export our code only if the user has specifically asked
// for it by defining ASIO_DYN_LINK.
#  if defined(NETWORK_DYNAMIC_LINK)
// Export if this is our own source, otherwise import.
#   if defined(NETWORK_SOURCE)
#    define NETWORK_API DLL_EXPORT
#   else // defined(NETWORK_SOURCE)  
#    define NETWORK_API DLL_IMPORT
#   endif // defined(NETWORK_SOURCE)
#  endif // defined(NETWORK_DYNAMIC_LINK)
# endif // defined(_MSC_VER) || defined(__BORLANDC__) || defined(__CODEGEARC__)
#endif // defined(NETWORK_HEADER_ONLY)


// If NETWORK_API isn't defined yet define it now.
#if !defined(NETWORK_API)
# define NETWORK_API
#endif // !defined(NETWORK_API)

// Linux: epoll, eventfd and timerfd.
#if defined(__linux__)
# include <linux/version.h>
# if !defined(NETWORK_HAS_EPOLL)
#  if !defined(NETWORK_DISABLE_EPOLL)
#   if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,45)
#    define NETWORK_HAS_EPOLL 1
#   endif // LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,45)
#  endif // !defined(NETWORK_DISABLE_EPOLL)
# endif // !defined(NETWORK_HAS_EPOLL)

# if !defined(NETWORK_HAS_POLL)
#  if !defined(NETWORK_HAS_EPOLL)
#   define NETWORK_HAS_POLL 1
#  endif // !defined(NETWORK_HAS_EPOLL)
# endif // !defined(NETWORK_HAS_POLL)

# if !defined(NETWORK_HAS_EVENTFD)
#  if !defined(NETWORK_DISABLE_EVENTFD)
#   if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22)
#    define NETWORK_HAS_EVENTFD 1
#   endif // LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22)
#  endif // !defined(NETWORK_DISABLE_EVENTFD)
# endif // !defined(NETWORK_HAS_EVENTFD)

# if !defined(NETWORK_HAS_TIMERFD)
#  if defined(NETWORK_HAS_EPOLL)
#   if (__GLIBC__ > 2) || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 8)
#    define NETWORK_HAS_TIMERFD 1
#   endif // (__GLIBC__ > 2) || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 8)
#  endif // defined(NETWORK_HAS_EPOLL)
# endif // !defined(NETWORK_HAS_TIMERFD)
#endif // defined(__linux__)


#endif // END OF NETLITE_CONFIG_HPP
