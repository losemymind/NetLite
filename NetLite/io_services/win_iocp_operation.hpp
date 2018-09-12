/****************************************************************************
  Copyright (c) 2018 libo All rights reserved.
  
  losemymind.libo@gmail.com

****************************************************************************/
#ifndef NETLITE_WIN_IOCP_OPERATION_HPP
#define NETLITE_WIN_IOCP_OPERATION_HPP

#include <windows.h>
#include <system_error>
#include <functional>
class win_iocp_operation : public OVERLAPPED
{
public:
    typedef std::function<void(void*, win_iocp_operation*, const std::error_code& , size_t)>  func_type;
    win_iocp_operation(const func_type& func)
        : callback(func)
    {
        reset();
    }

    // Prevents deletion through this type.
    ~win_iocp_operation()
    {

    }

    void complete(void* context, const std::error_code& ec, size_t bytes_transferred)
    {
        callback(context, this, ec, bytes_transferred);
    }

    void destroy()
    {
        callback(0, this, std::error_code(), 0);
    }
protected:
    void reset()
    {
        Internal     = 0;
        InternalHigh = 0;
        Offset       = 0;
        OffsetHigh   = 0;
        hEvent       = NULL;
    }
private:
    func_type  callback;

};

#endif // END OF NETLITE_WIN_IOCP_OPERATION_HPP