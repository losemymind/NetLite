

#include <system_error>
#include <atomic>
#include <queue>
#include <windows.h>
#include "win_iocp_operation.hpp"
class win_iocp_io_context
{
public:
    /// Constructor. Specifies a concurrency hint that is passed through to the
    /// underlying I/O completion port.
    win_iocp_io_context(int concurrency_hint = -1);
    virtual~win_iocp_io_context();
    void shutdown();

    /// Register a handle with the IO completion port.
    std::error_code register_handle(HANDLE handle, std::error_code& ec);

    
    // Run the event loop until stopped or no more work.
    size_t run(std::error_code& ec);

    // Run until stopped or one operation is performed.
    size_t run_one(std::error_code& ec);

    // Run until timeout, interrupted, or one operation is performed.
    size_t wait_one(long usec, std::error_code& ec);

    // Poll for operations without blocking.
    size_t poll(std::error_code& ec);

    // Poll for one operation without blocking.
    size_t poll_one(std::error_code& ec);

    // Stop the event processing loop.
    void stop();

    // Determine whether the io_context is stopped.
    bool stopped() const;
private:
    /// Dequeues at most one operation from the I/O completion port, and then
    /// executes it. Returns the number of operations that were dequeued (i.e.
    /// either 0 or 1).
    size_t do_one(DWORD msec, std::error_code& ec);

private:
    // Helper class for managing a HANDLE.
    struct auto_handle
    {
        HANDLE handle;
        auto_handle() : handle(0) {}
        ~auto_handle() { if (handle) ::CloseHandle(handle); }
    };


    // The IO completion port used for queueing operations.
    auto_handle        iocp_;
    int                concurrency_hint_;
    // Flag to indicate whether the service has been shut down.
    std::atomic_bool   shutdown_;

    // The count of unfinished work.
    std::atomic_long unfinished_work_;

    // Timeout to use with GetQueuedCompletionStatus.
    const DWORD gqcs_timeout_;

    // Flag to indicate whether the event loop has been stopped.
    mutable std::atomic_bool stopped_;

    // Flag to indicate whether there is an in-flight stop event. Every event
    // posted using PostQueuedCompletionStatus consumes non-paged pool, so to
    // avoid exhausting this resouce we limit the number of outstanding events.
    std::atomic_bool stop_event_posted_;

    // Non-zero if timers or completed operations need to be dispatched.
    std::atomic_long dispatch_required_;

    // The operations that are ready to dispatch.
    std::queue<win_iocp_operation*> completed_ops_;
};

