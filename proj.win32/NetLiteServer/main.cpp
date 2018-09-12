#include "NetLite/NetLite.hpp"
#include <unordered_map>
#include <thread>
#include <chrono>
#include <iostream>
int main()
{

    NetLite::tcp::socket  ServerSocket;
    NetLite::tcp::endpoint endpoint(NetLite::ip::address::from_string("127.0.0.1"), 9090);
    ServerSocket.open();
    ServerSocket.bind(endpoint);
    ServerSocket.listen();
    std::unordered_map<int, NetLite::tcp::socket> Clients;
    int ClientIndex = 0;
    std::string ServerMsg = "I recv your connect.";
    while (true)
    {
        NetLite::tcp::endpoint RemoteEndpoint;
        NetLite::tcp::socket ClientSocket = ServerSocket.accept(RemoteEndpoint);
        Clients.insert(std::make_pair(ClientIndex++, ClientSocket));
        std::cout << "Client connected:" << ClientSocket.native_handle() << " ipaddress:" << ClientSocket.remote_endpoint().address().to_string() << "port:"<< ClientSocket.remote_endpoint().port() <<std::endl;
        std::vector<char> SendBuffer;
        SendBuffer.resize(ServerMsg.size());
        memcpy(SendBuffer.data(), ServerMsg.data(), ServerMsg.size());
        ClientSocket.send(SendBuffer);
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    return 0;
}