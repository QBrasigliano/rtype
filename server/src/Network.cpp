#include "../include/Network.hpp"

#include <iostream>

NetworkManager::NetworkManager(uint16_t port) 
    : port_(port), next_client_id_(1) {
}

NetworkManager::~NetworkManager() {
    Stop();
}

void NetworkManager::Start() {
    try {
        acceptor_ = std::make_unique<tcp::acceptor>(
            io_,
            tcp::endpoint(tcp::v4(), port_)
        );
        
        std::cout << "Serveur écoute sur le port " << port_ << std::endl;
        
        AcceptNextClient();
    } catch (std::exception& e) {
        std::cerr << "Erreur serveur: " << e.what() << std::endl;
    }
}

void NetworkManager::AcceptNextClient() {
    if (!acceptor_ || !acceptor_->is_open())
        return;
    
    // nouvelle socket client
    auto socket = std::make_unique<tcp::socket>(io_);
    
    // async_accept -> quand un client se connecte, appel lambda
    acceptor_->async_accept(*socket, 
        [this, socket = std::move(socket)](const asio::error_code& error) mutable {
            if (!error) {
                int client_id = next_client_id_++;
                std::cout << "Client ID: " << client_id << " connecté" << std::endl;
                
                // cree shared_ptr pour socket
                auto shared_socket = std::make_shared<tcp::socket>(std::move(*socket));
                ReadFromClient(shared_socket, client_id);
            } else {
                std::cerr << "Erreur connexion: " << error.message() << std::endl;
            }

            AcceptNextClient();
        }
    );
}

void NetworkManager::ReadFromClient(std::shared_ptr<tcp::socket> socket, int client_id) {
    // Créer un buffer pour lire l'en-tête du paquet (header + size = 4 bytes)
    auto header_buffer = std::make_shared<std::vector<uint8_t>>(4);
    
    // Lecture asynchrone de l'en-tête
    asio::async_read(*socket, asio::buffer(*header_buffer),
        [this, socket, header_buffer, client_id](const asio::error_code& read_error, std::size_t /*bytes*/) mutable {
            if (!read_error) {
                // Vérifier le magic header (0xDEAD)
                uint16_t header = (static_cast<uint16_t>((*header_buffer)[0]) << 8) | (*header_buffer)[1];
                if (header != PACKET_HEADER) {
                    return;
                }
                
                // Lire la taille du payload
                uint16_t payload_size = (static_cast<uint16_t>((*header_buffer)[2]) << 8) | (*header_buffer)[3];
                
                // Créer un buffer pour le reste : type(1) + data(payload_size) + checksum(2)
                auto payload_buffer = std::make_shared<std::vector<uint8_t>>(1 + payload_size + 2);
                
                // Lire le payload
                asio::async_read(*socket, asio::buffer(*payload_buffer),
                    [this, socket, header_buffer, payload_buffer, client_id](const asio::error_code& payload_error, std::size_t /*bytes*/) mutable {
                        if (!payload_error) {
                            // Reconstruire le paquet complet
                            std::vector<uint8_t> complete_packet;
                            complete_packet.insert(complete_packet.end(), header_buffer->begin(), header_buffer->end());
                            complete_packet.insert(complete_packet.end(), payload_buffer->begin(), payload_buffer->end());
                            
                            // Déserialiser le paquet
                            Packet packet;
                            if (Packet::Deserialize(complete_packet, packet)) {
                                // Traiter le paquet du client
                            }
                            
                            // Relire le prochain paquet du même client
                            ReadFromClient(socket, client_id);
                        }
                    }
                );
            } else {
                // "End of file" = client déconnecté proprement
                if (read_error == asio::error::eof) {
                    std::cout << "Client ID: " << client_id << " déconnecté" << std::endl;
                }
            }
        }
    );
}

void NetworkManager::Stop() {
    if (acceptor_)
        acceptor_->close();
    io_.stop();
}

// notif client
void NetworkManager::Update() {
    io_.poll_one();
}

void NetworkManager::SendToClient(int clientId, const Packet& packet) {
    // à implémenter
    std::cout << "Envoi paquet au client " << clientId << std::endl;
}
