#include "NetLite/NetLite.hpp"
#include <unordered_map>
#include <thread>
#include <chrono>
#include <iostream>
int main()
{

    NetLite::tcp::socket  ClientSocket;
    ClientSocket.open();
    NetLite::tcp::endpoint endpoint(NetLite::ip::address::from_string("127.0.0.1"), 9090);
    ClientSocket.connect(endpoint);
    while (true)
    {
        ClientSocket.wait(NetLite::socket_base::wait_read);
        std::vector<char> Buffer;
        Buffer.resize(1024);
        ClientSocket.receive(Buffer);
        std::cout << Buffer.data() << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    return 0;
}