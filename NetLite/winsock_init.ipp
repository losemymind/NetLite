
#ifndef NETLITE_WINSOCK_INIT_IPP
#define NETLITE_WINSOCK_INIT_IPP

#include "NetLite/socket_types.hpp"
#include "NetLite/winsock_init.hpp"

#if defined(_WIN32)

void winsock_init_base::startup(data& d,
    unsigned char major, unsigned char minor)
{
  if (::InterlockedIncrement(&d.init_count_) == 1)
  {
    WSADATA wsa_data;
    long result = ::WSAStartup(MAKEWORD(major, minor), &wsa_data);
    ::InterlockedExchange(&d.result_, result);
  }
}

void winsock_init_base::cleanup(data& d)
{
  if (::InterlockedDecrement(&d.init_count_) == 0)
  {
    ::WSACleanup();
  }
}

void winsock_init_base::throw_on_error(data& d)
{
  long result = ::InterlockedExchangeAdd(&d.result_, 0);
  if (result != 0)
  {

  }
}


#endif //defined(_WIN32)

#endif // END OF NETLITE_WINSOCK_INIT_IPP
