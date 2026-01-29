#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <asio.hpp>
#include <vector>
#include <memory>
#include "../include/Protocol.hpp"

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
    void AcceptNextClient();
};

#endif // NETWORK_HPP