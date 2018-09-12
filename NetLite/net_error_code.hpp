
#ifndef NETLITE_NET_ERROR_CODE_HPP
#define NETLITE_NET_ERROR_CODE_HPP


#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <system_error>
#if defined(_WIN32) || defined(__CYGWIN__)
# include <winerror.h>
#else
# include <cerrno>
# include <netdb.h>
#endif


#if defined(_WIN32) || defined(__CYGWIN__)
# define NET_NATIVE_ERROR(e) e
# define NET_SOCKET_ERROR(e) WSA ## e
# define NET_NETDB_ERROR(e) WSA ## e
# define NET_GETADDRINFO_ERROR(e) WSA ## e
# define NET_WIN_OR_POSIX(e_win, e_posix) e_win
#else
# define NET_NATIVE_ERROR(e) e
# define NET_SOCKET_ERROR(e) e
# define NET_NETDB_ERROR(e) e
# define NET_GETADDRINFO_ERROR(e) e
# define NET_WIN_OR_POSIX(e_win, e_posix) e_posix
#endif

#if defined(_WIN32)
struct _Map_errtab_t
{	// maps Windows error_code to STL generic_errno.
    int _Win_Errcode;
    std::errc _STD_Errcode;
};

static const _Map_errtab_t _Map_errtab[] =
{	// table of Windows code/name pairs
    ERROR_ACCESS_DENIED, std::errc::permission_denied,
    ERROR_ALREADY_EXISTS, std::errc::file_exists,
    ERROR_BAD_UNIT, std::errc::no_such_device,
    ERROR_BUFFER_OVERFLOW, std::errc::filename_too_long,
    ERROR_BUSY, std::errc::device_or_resource_busy,
    ERROR_BUSY_DRIVE, std::errc::device_or_resource_busy,
    ERROR_CANNOT_MAKE, std::errc::permission_denied,
    ERROR_CANTOPEN, std::errc::io_error,
    ERROR_CANTREAD, std::errc::io_error,
    ERROR_CANTWRITE, std::errc::io_error,
    ERROR_CURRENT_DIRECTORY, std::errc::permission_denied,
    ERROR_DEV_NOT_EXIST, std::errc::no_such_device,
    ERROR_DEVICE_IN_USE, std::errc::device_or_resource_busy,
    ERROR_DIR_NOT_EMPTY, std::errc::directory_not_empty,
    ERROR_DIRECTORY, std::errc::invalid_argument,
    ERROR_DISK_FULL, std::errc::no_space_on_device,
    ERROR_FILE_EXISTS, std::errc::file_exists,
    ERROR_FILE_NOT_FOUND, std::errc::no_such_file_or_directory,
    ERROR_HANDLE_DISK_FULL, std::errc::no_space_on_device,
    ERROR_INVALID_ACCESS, std::errc::permission_denied,
    ERROR_INVALID_DRIVE, std::errc::no_such_device,
    ERROR_INVALID_FUNCTION, std::errc::function_not_supported,
    ERROR_INVALID_HANDLE, std::errc::invalid_argument,
    ERROR_INVALID_NAME, std::errc::invalid_argument,
    ERROR_LOCK_VIOLATION, std::errc::no_lock_available,
    ERROR_LOCKED, std::errc::no_lock_available,
    ERROR_NEGATIVE_SEEK, std::errc::invalid_argument,
    ERROR_NOACCESS, std::errc::permission_denied,
    ERROR_NOT_ENOUGH_MEMORY, std::errc::not_enough_memory,
    ERROR_NOT_READY, std::errc::resource_unavailable_try_again,
    ERROR_NOT_SAME_DEVICE, std::errc::cross_device_link,
    ERROR_OPEN_FAILED, std::errc::io_error,
    ERROR_OPEN_FILES, std::errc::device_or_resource_busy,
    ERROR_OPERATION_ABORTED, std::errc::operation_canceled,
    ERROR_OUTOFMEMORY, std::errc::not_enough_memory,
    ERROR_PATH_NOT_FOUND, std::errc::no_such_file_or_directory,
    ERROR_READ_FAULT, std::errc::io_error,
    ERROR_RETRY, std::errc::resource_unavailable_try_again,
    ERROR_SEEK, std::errc::io_error,
    ERROR_SHARING_VIOLATION, std::errc::permission_denied,
    ERROR_TOO_MANY_OPEN_FILES, std::errc::too_many_files_open,
    ERROR_WRITE_FAULT, std::errc::io_error,
    ERROR_WRITE_PROTECT, std::errc::permission_denied,
    WSAEACCES, std::errc::permission_denied,
    WSAEADDRINUSE, std::errc::address_in_use,
    WSAEADDRNOTAVAIL, std::errc::address_not_available,
    WSAEAFNOSUPPORT, std::errc::address_family_not_supported,
    WSAEALREADY, std::errc::connection_already_in_progress,
    WSAEBADF, std::errc::bad_file_descriptor,
    WSAECONNABORTED, std::errc::connection_aborted,
    WSAECONNREFUSED, std::errc::connection_refused,
    WSAECONNRESET, std::errc::connection_reset,
    WSAEDESTADDRREQ, std::errc::destination_address_required,
    WSAEFAULT, std::errc::bad_address,
    WSAEHOSTUNREACH, std::errc::host_unreachable,
    WSAEINPROGRESS, std::errc::operation_in_progress,
    WSAEINTR, std::errc::interrupted,
    WSAEINVAL, std::errc::invalid_argument,
    WSAEISCONN, std::errc::already_connected,
    WSAEMFILE, std::errc::too_many_files_open,
    WSAEMSGSIZE, std::errc::message_size,
    WSAENAMETOOLONG, std::errc::filename_too_long,
    WSAENETDOWN, std::errc::network_down,
    WSAENETRESET, std::errc::network_reset,
    WSAENETUNREACH, std::errc::network_unreachable,
    WSAENOBUFS, std::errc::no_buffer_space,
    WSAENOPROTOOPT, std::errc::no_protocol_option,
    WSAENOTCONN, std::errc::not_connected,
    WSAENOTSOCK, std::errc::not_a_socket,
    WSAEOPNOTSUPP, std::errc::operation_not_supported,
    WSAEPROTONOSUPPORT, std::errc::operation_not_supported,
    WSAEPROTOTYPE, std::errc::wrong_protocol_type,
    WSAETIMEDOUT, std::errc::timed_out,
    WSAEWOULDBLOCK, std::errc::operation_would_block,
    0, std::errc::operation_not_supported
};

static inline int  WinErrorCodeToErrc(int _Errcode)
{
    const _Map_errtab_t *_Ptr = &_Map_errtab[0];
    for (; _Ptr->_Win_Errcode != 0; ++_Ptr)
        if ((int)_Ptr->_Win_Errcode == _Errcode)
            return static_cast<int>(_Ptr->_STD_Errcode);
    return _Errcode;
}

#endif //#if defined(_WIN32)

namespace NetLite
{
    inline void throw_if(const std::error_code& err, const char* file = "", int line = 0)
    {
        if (err)
        {
            std::string what_msg = file;
            if (!what_msg.empty())
            {
                what_msg += "(";
                what_msg += std::to_string(line);
                what_msg += "):";
            }
            what_msg += err.message();
            std::system_error e(err, what_msg);
            throw e;
        }
    }
}



#endif // END OF NETLITE_NET_ERROR_CODE_HPP
