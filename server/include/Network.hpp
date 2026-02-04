#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <asio.hpp>
#include <vector>
#include <memory>
#include "Protocol.hpp"
#include "ClientRegistry.hpp"

using asio::ip::tcp;

class NetworkManager {
public:
    NetworkManager(uint16_t port);
    ~NetworkManager();

    void Start();
    void Stop();
    void SendToClient(int clientId, const Packet& packet);
    void Update();
    
private:
    asio::io_context io_;
    std::unique_ptr<tcp::acceptor> acceptor_;
    uint16_t port_;
    int next_client_id_;
    ClientRegistry registry_;
    void AcceptNextClient();
    void ReadFromClient(std::shared_ptr<tcp::socket> socket, int client_id);
};

#endif // NETWORK_HPP