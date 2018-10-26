#include "NetLite/tcp.hpp"
#include "NetLite/mutablebuf.hpp"
#include <unordered_map>
#include <thread>
#include <chrono>
#include <iostream>
int main()
{

    NetLite::tcp::socket  ClientSocket;
    ClientSocket.open();
    ClientSocket.non_blocking(true);
    NetLite::tcp::endpoint endpoint(NetLite::ip::address::from_string("127.0.0.1"), 9090);
    ClientSocket.connect(endpoint);
    std::string msg;
    std::thread  input_thread([&] 
    {
        while (true)
        {
            std::cin >> msg;
        }
    });
    input_thread.detach();
    std::error_code ec;
    while (true)
    {
        if (ClientSocket.has_state(NetLite::socket_base::readable, ec) == NetLite::tcp::socket::state_return::Yes)
        {
            std::vector<char> Buffer;
            Buffer.resize(1024);
            ClientSocket.receive(NetLite::make_mutablebuf(Buffer));
            std::cout << Buffer.data() << "\n" << std::endl;
        }

        if (ClientSocket.has_state(NetLite::socket_base::writable, ec) == NetLite::tcp::socket::state_return::Yes)
        {
            if (!msg.empty())
            {
                ClientSocket.send(NetLite::make_constbuf(msg));
                msg.clear();
            }
        }


        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    return 0;
}