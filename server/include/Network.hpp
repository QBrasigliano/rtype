#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <asio.hpp>
#include <vector>
#include <memory>
#include "Protocol.hpp"
#include "ClientRegistry.hpp"
#include "World.hpp"

using asio::ip::tcp;

class NetworkManager {
public:
    NetworkManager(uint16_t port);
    ~NetworkManager();

    void Start();
    void Stop();
    void SendToClient(int clientId, const Packet& packet);
    void Update();
    World& GetWorld() { return world_; }
    ClientRegistry& GetRegistry() { return registry_; }
    
private:
    asio::io_context io_;
    std::unique_ptr<tcp::acceptor> acceptor_;
    uint16_t port_;
    int next_client_id_;
    ClientRegistry registry_;
    World world_;
    int next_bullet_id_ = 1;
    void AcceptNextClient();
    void ReadFromClient(std::shared_ptr<tcp::socket> socket, int client_id);
};

#endif // NETWORK_HPP