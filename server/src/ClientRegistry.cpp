#include "ClientRegistry.hpp"
#include <iostream>

void ClientRegistry::AddClient(int client_id, std::shared_ptr<tcp::socket> socket) {
    clients_[client_id] = socket;
    std::cout << "✅ Client ID: " << client_id << " ajouté au registre" << std::endl;
}

void ClientRegistry::RemoveClient(int client_id) {
    auto it = clients_.find(client_id);
    if (it != clients_.end()) {
        clients_.erase(it);
        std::cout << "✅ Client ID: " << client_id << " supprimé du registre" << std::endl;
    }
}

void ClientRegistry::SendToAll(const Packet& packet) {
    auto bytes = packet.Serialize();
    for (auto& [id, socket] : clients_) {
        if (socket && socket->is_open()) {
            socket->async_write_some(asio::buffer(bytes),
                [id](const asio::error_code& ec, std::size_t /*bytes_transferred*/) {
                    if (ec) {
                        std::cerr << "❌ Erreur envoi à client " << id << ": " << ec.message() << std::endl;
                    }
                });
        }
    }
}

bool ClientRegistry::HasClient(int client_id) const {
    return clients_.find(client_id) != clients_.end();
}

std::shared_ptr<tcp::socket> ClientRegistry::GetSocket(int client_id) {
    auto it = clients_.find(client_id);
    if (it != clients_.end()) {
        return it->second;
    }
    return nullptr;
}
