#include "NetLite/tcp.hpp"
#include "NetLite/mutablebuf.hpp"
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
    ServerSocket.non_blocking(true);
    std::unordered_map<int, NetLite::tcp::socket> Clients;
    int ClientIndex = 0;
    std::string ServerMsg = "I recv your connect.\n";
    std::unordered_map<int, std::string> ClientMsgs;
    while (true)
    {
        std::error_code ec;
        if (ServerSocket.has_pending_accept(ec))
        {
            NetLite::tcp::endpoint RemoteEndpoint;
            NetLite::tcp::socket ClientSocket = ServerSocket.accept(RemoteEndpoint);
            ClientSocket.non_blocking(true);
            Clients.insert(std::make_pair(ClientIndex++, ClientSocket));
            std::cout << "Client connected:" << ClientSocket.native_handle() << " ipaddress:" << ClientSocket.remote_endpoint().address().to_string() << "port:" << ClientSocket.remote_endpoint().port() << std::endl;

        }

        for (auto& client : Clients)
        {
            if (client.second.has_state(NetLite::socket_base::readable, ec)== NetLite::tcp::socket::state_return::Yes && client.second.available() > 0)
            {
                std::vector<char> Buffer;
                Buffer.resize(1024);
                client.second.receive(NetLite::make_mutablebuf(Buffer));
                std::cout << client.second.remote_endpoint().address().to_string()<<":"<<Buffer.data() << "\n" << std::endl;
                char cid[2] = { 0 };
                cid[0] = Buffer[0];
                int clientid = atoi(cid);
                ClientMsgs[clientid] = std::string(&Buffer[1], Buffer.size() - 1);
            }

            if (client.second.has_state(NetLite::socket_base::writable, ec) == NetLite::tcp::socket::state_return::Yes)
            {
                for (auto& msg : ClientMsgs)
                {
                    if (msg.first == client.first)
                    {
                        client.second.send(NetLite::make_constbuf(msg.second));
                        ClientMsgs[msg.first].clear();
                    }
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    return 0;
}