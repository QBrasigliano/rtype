#include "ClientRegistry.hpp"
#include <iostream>

void ClientRegistry::AddClient(int client_id, std::shared_ptr<tcp::socket> socket) {
    clients_[client_id] = socket;       // add client to map
}

void ClientRegistry::RemoveClient(int client_id) {
    auto it = clients_.find(client_id);     // find client
    if (it != clients_.end()) {
        clients_.erase(it);                 // sup client de map
    }
}

void ClientRegistry::SendToAll(const Packet& packet) {
    auto bytes = packet.Serialize();                        // convert packet to bytes
    for (auto& [id, socket] : clients_) {
        if (socket && socket->is_open()) {                  // check socket valid
            socket->async_write_some(asio::buffer(bytes),       //send
                [id](const asio::error_code& ec, std::size_t /*bytes_transferred*/) {
                    if (ec)
                        std::cerr << "❌ Erreur envoi à client " << id << ": " << ec.message() << std::endl;
                });
        }
    }
}

bool ClientRegistry::HasClient(int client_id) const {
    return clients_.find(client_id) != clients_.end();
}

std::shared_ptr<tcp::socket> ClientRegistry::GetSocket(int client_id) {
    auto it = clients_.find(client_id);         // find client
    if (it != clients_.end())                   // if found
        return it->second;                      // return socket
    return nullptr;
}
