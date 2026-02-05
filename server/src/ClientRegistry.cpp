#include "ClientRegistry.hpp"
#include <iostream>

void ClientRegistry::AddClient(int client_id, std::shared_ptr<tcp::socket> socket) {
    clients_[client_id] = socket;
    std::cout << "✅ Client ID: " << client_id << " ajouté au registre" << std::endl;
    
    // Envoyer pos spawn
    Packet spawn_packet;
    spawn_packet.header = PACKET_HEADER;
    spawn_packet.size = 5;  // client_id(1) + X(2) + Y(2)
    spawn_packet.type = PacketType::PLAYER_MOVED;
    spawn_packet.data = {
        static_cast<uint8_t>(client_id),
        static_cast<uint8_t>(100 >> 8),   // X = 100
        static_cast<uint8_t>(100 & 0xFF),
        static_cast<uint8_t>(400 >> 8),   // Y = 400
        static_cast<uint8_t>(400 & 0xFF)
    };
    spawn_packet.checksum = spawn_packet.CalculateChecksum();
    
    // Envoyer à tous les autres clients (pas au nouveau)
    SendToAllExcept(spawn_packet, client_id);
}

void ClientRegistry::RemoveClient(int client_id) {
    auto it = clients_.find(client_id);
    // 🔍 Vérifier si le client existe vraiment dans le registre avant de le supprimer
    if (it != clients_.end()) {
        clients_.erase(it);
        std::cout << "✅ Client ID: " << client_id << " supprimé du registre" << std::endl;
    }
}

void ClientRegistry::SendToAll(const Packet& packet) {
    auto bytes = std::make_shared<std::vector<uint8_t>>(packet.Serialize());  // convert packet to bytes
    for (auto& [id, socket] : clients_) {
        // 🔌 Vérifier que la socket existe ET qu'elle est ouverte avant d'envoyer
        if (socket && socket->is_open()) {                  // check socket valid
            asio::async_write(*socket, asio::buffer(*bytes),       //send TOUT
                [id, bytes](const asio::error_code& ec, std::size_t bytes_sent) {
                    // ⚠️ Vérifier s'il y a eu une erreur lors de l'envoi
                    if (ec)
                        std::cerr << "❌ Erreur envoi à client " << id << ": " << ec.message() << std::endl;
                });
        }
    }
}

void ClientRegistry::SendToAllExcept(const Packet& packet, int exclude_client_id) {
    auto bytes = std::make_shared<std::vector<uint8_t>>(packet.Serialize());
    for (auto& [id, socket] : clients_) {
        // 🚫 Vérifier que ce n'est PAS le client à exclure ET que la socket est valide
        if (id != exclude_client_id && socket && socket->is_open()) {
            asio::async_write(*socket, asio::buffer(*bytes),
                [id, bytes](const asio::error_code& ec, std::size_t bytes_sent) {
                    // ⚠️ Vérifier s'il y a eu une erreur lors de l'envoi (exclusion)
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
    auto it = clients_.find(client_id);
    // 🔍 Vérifier si le client existe dans le registre pour retourner sa socket
    if (it != clients_.end()) {
        return it->second;
    }
    return nullptr;
}
