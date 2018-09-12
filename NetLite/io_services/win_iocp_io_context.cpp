#include "win_iocp_io_context.hpp"


namespace {

    DWORD get_gqcs_timeout()
    {
        const DWORD default_gqcs_timeout = 500;
        OSVERSIONINFOEX osvi;
        ZeroMemory(&osvi, sizeof(osvi));
        osvi.dwOSVersionInfoSize = sizeof(osvi);
        osvi.dwMajorVersion = 6ul;

        const uint64_t condition_mask = ::VerSetConditionMask(
            0, VER_MAJORVERSION, VER_GREATER_EQUAL);

        if (!!::VerifyVersionInfo(&osvi, VER_MAJORVERSION, condition_mask))
            return INFINITE;

        return default_gqcs_timeout;
    }
}

win_iocp_io_context::win_iocp_io_context(int concurrency_hint /*= -1*/)
    : concurrency_hint_(concurrency_hint)
    , shutdown_(false)
    , unfinished_work_(0)
    , stopped_(false)
    , stop_event_posted_(false)
    , gqcs_timeout_(get_gqcs_timeout())
{
    iocp_.handle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0,
        static_cast<DWORD>(concurrency_hint >= 0 ? concurrency_hint : DWORD(~0)));
    if (!iocp_.handle)
    {
        DWORD last_error = ::GetLastError();
        std::error_code ec(last_error, std::system_category());
        throw ec;
    }
}

win_iocp_io_context::~win_iocp_io_context()
{

}

void win_iocp_io_context::shutdown()
{
    shutdown_ = true;
    while (unfinished_work_ > 0)
    {
        std::queue<win_iocp_operation*> ops;
        ops = completed_ops_;
        if (!ops.empty())
        {
            while (win_iocp_operation* op = ops.front())
            {
                ops.pop();
                --unfinished_work_;
                op->destroy();
            }
        }
        else
        {
            DWORD bytes_transferred = 0;
            DWORD_PTR completion_key = 0;
            LPOVERLAPPED overlappend = 0;
            ::GetQueuedCompletionStatus(iocp_.handle, &bytes_transferred, &completion_key, &overlappend, gqcs_timeout_);
            if (overlappend)
            {
                --unfinished_work_;
                static_cast<win_iocp_operation*>(overlappend)->destroy();
            }
        }
    }

}

std::error_code win_iocp_io_context::register_handle(HANDLE handle, std::error_code& ec)
{
    if (::CreateIoCompletionPort(handle, iocp_.handle, 0, 0) == 0)
    {
        DWORD last_error = ::GetLastError();
        ec = std::error_code(last_error, std::system_category());
    }
    else
    {
        ec = std::error_code();
    }
    return ec;
}

size_t win_iocp_io_context::run(std::error_code& ec)
{
    if (unfinished_work_ == 0)
    {
        stop();
        ec = std::error_code();
        return 0;
    }

    size_t n = 0;
    while (do_one(INFINITE, ec))
    {
        if (n != (std::numeric_limits<size_t>::max)())
        {
            ++n;
        }
    }
    return n;
}

size_t win_iocp_io_context::run_one(std::error_code& ec)
{
    if (unfinished_work_ == 0)
    {
        stop();
        ec = std::error_code();
        return 0;
    }

    return do_one(INFINITE, ec);
}

size_t win_iocp_io_context::wait_one(long usec, std::error_code& ec)
{
    if (unfinished_work_ == 0)
    {
        stop();
        ec = std::error_code();
        return 0;
    }

    return do_one(usec < 0 ? INFINITE : ((usec - 1) / 1000 + 1), ec);
}

size_t win_iocp_io_context::poll(std::error_code& ec)
{
    if (unfinished_work_ == 0)
    {
        stop();
        ec = std::error_code();
        return 0;
    }

    size_t n = 0;
    while (do_one(0, ec))
    {
        if (n != (std::numeric_limits<size_t>::max)())
        {
            ++n;
        }
    }
    return n;
}

size_t win_iocp_io_context::poll_one(std::error_code& ec)
{
    if (unfinished_work_ == 0)
    {
        stop();
        ec = std::error_code();
        return 0;
    }
    return do_one(0, ec);
}

void win_iocp_io_context::stop()
{
    if (stopped_ == false)
    {
        stopped_ = true;
        if (stop_event_posted_ == false)
        {
            stop_event_posted_ = true;
            if (!::PostQueuedCompletionStatus(iocp_.handle,0,0,0))
            {
                DWORD last_error = ::GetLastError();
                std::error_code ec(last_error, std::system_category());
                throw ec;
            }
        }
    }
}

bool win_iocp_io_context::stopped() const
{
    return stopped_.load();
}

size_t win_iocp_io_context::do_one(DWORD msec, std::error_code& ec)
{
    for (;;)
    {

    }
}