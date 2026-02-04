#pragma once

#include <map>
#include <memory>
#include <asio.hpp>
#include "Protocol.hpp"

using asio::ip::tcp;

class ClientRegistry {
public:
    ClientRegistry() = default;
    ~ClientRegistry() = default;

    void AddClient(int client_id, std::shared_ptr<tcp::socket> socket);
    void RemoveClient(int client_id);
    void SendToAll(const Packet& packet);
    
    bool HasClient(int client_id) const;
    std::shared_ptr<tcp::socket> GetSocket(int client_id);
    
private:
    std::map<int, std::shared_ptr<tcp::socket>> clients_;       // map client_id -> socket
};
