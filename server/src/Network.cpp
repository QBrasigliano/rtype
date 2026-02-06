#include "../include/Network.hpp"
#include "../include/ClientRegistry.hpp"
#include "Component.hpp"
#include "Entity.hpp"
#include "World.hpp"
#include "Components/Position.hpp"
#include "Components/Velocity.hpp"
#include "Components/Bullet.hpp"
#include "Systems/PhysicsSystem.hpp"

#include <iostream>
#include <vector>


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
    // 🔍 Vérifier si l'accepteur TCP est initialisé et ouvert pour accepter des connexions
    if (!acceptor_ || !acceptor_->is_open())
        return;
    
    // nouvelle socket client
    auto socket = std::make_unique<tcp::socket>(io_);
    
    // async_accept -> quand un client se connecte, appel lambda
    acceptor_->async_accept(*socket, 
        [this, socket = std::move(socket)](const asio::error_code& error) mutable {
            // ✅ Vérifier si la connexion du client s'est bien passée (pas d'erreur ASIO)
            if (!error) {
                int client_id = next_client_id_++;
                std::cout << "Client ID: " << client_id << " connecté" << std::endl;
                
                // cree shared_ptr pour socket
                auto shared_socket = std::make_shared<tcp::socket>(std::move(*socket));
                registry_.AddClient(client_id, shared_socket);
                ReadFromClient(shared_socket, client_id);
            } else {
                std::cerr << "Erreur connexion: " << error.message() << std::endl;
            }

            AcceptNextClient();
        }
    );
}

void NetworkManager::ReadFromClient(std::shared_ptr<tcp::socket> socket, int client_id) {
    // nouveau buffer pour lire l entete
    auto header_buffer = std::make_shared<std::vector<uint8_t>>(4);
    
    // lecture ce celle la 
    asio::async_read(*socket, asio::buffer(*header_buffer),
        [this, socket, header_buffer, client_id](const asio::error_code& read_error, std::size_t bytes_read) mutable {
            // 📖 Vérifier si la lecture de l'en-tête du packet (4 bytes) s'est bien passée
            if (!read_error) {
                // Vérifier le header (0xDEAD)
                uint16_t header = (static_cast<uint16_t>((*header_buffer)[0]) << 8) | (*header_buffer)[1];
                // 🔒 Vérifier que le packet commence bien par 0xDEAD (sécurité protocole)
                if (header != PACKET_HEADER) {
                    std::cout << "Bad header: 0x" << std::hex << header << " pour client " << client_id << std::endl;
                    return;
                }
                
                // Lire la taille du payload
                uint16_t payload_size = (static_cast<uint16_t>((*header_buffer)[2]) << 8) | (*header_buffer)[3];
                
                // nouveau buffer pour le reste : type(1) + data(payload_size) + checksum(2)
                auto payload_buffer = std::make_shared<std::vector<uint8_t>>(1 + payload_size + 2);
                
                // Lire le payload
                asio::async_read(*socket, asio::buffer(*payload_buffer),
                    [this, socket, header_buffer, payload_buffer, client_id](const asio::error_code& payload_error, std::size_t payload_bytes) mutable {
                        // 📦 Vérifier si la lecture du contenu du packet (data + checksum) s'est bien passée
                        if (!payload_error) {
                            // Reconstruire le full packet
                            std::vector<uint8_t> complete_packet;
                            complete_packet.insert(complete_packet.end(), header_buffer->begin(), header_buffer->end());
                            complete_packet.insert(complete_packet.end(), payload_buffer->begin(), payload_buffer->end());
                            
                            // deserialiser paquet
                            Packet packet;
                            // ✅ Vérifier si le packet reçu est valide (checksum + structure correcte)
                            if (Packet::Deserialize(complete_packet, packet)) {
                                // traiter paquet selon type
                                if (packet.type == PacketType::SHOOT) {
                                    std::cout << "Client " << client_id << " tire!" << std::endl;
                                    
                                    // pos tir
                                    uint16_t shootX = 0, shootY = 0;
                                    // 📏 Vérifier que le packet contient bien les coordonnées X,Y (4 bytes minimum)
                                    if (packet.data.size() >= 4) {
                                        shootX = (static_cast<uint16_t>(packet.data[0]) << 8) | packet.data[1];
                                        shootY = (static_cast<uint16_t>(packet.data[2]) << 8) | packet.data[3];
                                    }
                                    
                                    // Create bullet entity in ECS
                                    auto bulletEntity = world_.CreateEntity();
                                    bulletEntity->AddComponent<Position>(static_cast<float>(shootX), static_cast<float>(shootY));
                                    bulletEntity->AddComponent<Velocity>(300.0f, 0.0f);
                                    bulletEntity->AddComponent<BulletComponent>(next_bullet_id_, client_id, 3.0f);
                                    
                                    int bullet_id = next_bullet_id_;
                                    next_bullet_id_++;
                                    
                                    // Broadcaster bullet aux clients
                                    Packet bulletBroadcast;
                                    bulletBroadcast.header = PACKET_HEADER;
                                    bulletBroadcast.size = 9;  // bullet_id(1) + x(2) + y(2) + vx(2) + vy(2)
                                    bulletBroadcast.type = PacketType::BULLET_SPAWNED;
                                    
                                    uint16_t encoded_x = shootX;
                                    uint16_t encoded_y = shootY;
                                    uint16_t encoded_vx = 300;
                                    uint16_t encoded_vy = static_cast<uint16_t>(0 + 32768); // Offset pour valeurs négatives
                                    
                                    bulletBroadcast.data = {
                                        static_cast<uint8_t>(bullet_id),
                                        static_cast<uint8_t>((encoded_x >> 8) & 0xFF),
                                        static_cast<uint8_t>(encoded_x & 0xFF),
                                        static_cast<uint8_t>((encoded_y >> 8) & 0xFF),
                                        static_cast<uint8_t>(encoded_y & 0xFF),
                                        static_cast<uint8_t>((encoded_vx >> 8) & 0xFF),
                                        static_cast<uint8_t>(encoded_vx & 0xFF),
                                        static_cast<uint8_t>((encoded_vy >> 8) & 0xFF),
                                        static_cast<uint8_t>(encoded_vy & 0xFF)
                                    };
                                    bulletBroadcast.checksum = bulletBroadcast.CalculateChecksum();
                                    
                                    registry_.SendToAll(bulletBroadcast);
                                    
                                    std::cout << "   Bullet créée: ID=" << bullet_id 
                                              << " pos=(" << shootX << "," << shootY 
                                              << ") vel=(300,0)" << std::endl;
                                    
                                } else {
                                    // new broadcast packet with position
                                    Packet broadcast;
                                    broadcast.header = PACKET_HEADER;
                                    broadcast.size = 5;  // client_id(1) + X(2) + Y(2) - no type
                                    broadcast.type = PacketType::PLAYER_MOVED;
                                    
                                    // pos client
                                    uint16_t playerX = 0, playerY = 0;
                                    // 📏 Vérifier que le packet contient bien les coordonnées X,Y du joueur (4 bytes minimum)
                                    if (packet.data.size() >= 4) {
                                        playerX = (static_cast<uint16_t>(packet.data[0]) << 8) | packet.data[1];
                                        playerY = (static_cast<uint16_t>(packet.data[2]) << 8) | packet.data[3];
                                    }
                                    
                                    // Créer le broadcast avec position absolue - SANS type à la fin
                                    broadcast.data = {
                                        static_cast<uint8_t>(client_id),
                                        static_cast<uint8_t>((playerX >> 8) & 0xFF),
                                        static_cast<uint8_t>(playerX & 0xFF),
                                        static_cast<uint8_t>((playerY >> 8) & 0xFF),
                                        static_cast<uint8_t>(playerY & 0xFF)
                                    };
                                    broadcast.checksum = broadcast.CalculateChecksum();
                                    
                                    // envoyer à tout les autre clients 
                                    registry_.SendToAllExcept(broadcast, client_id);
                                }
                            } else {
                                std::cout << "Erreur déserialisation pour client " << client_id << " - ARRÊT de la lecture (désynchronisation)" << std::endl;
                                return;
                            }
                            
                            // Relire le prochain paquet du même client
                            ReadFromClient(socket, client_id);
                        } else {
                            std::cout << "Erreur lecture payload pour client " << client_id << ": " << payload_error.message() << std::endl;
                        }
                    }
                );
            } else {
                // 🔌 Distinguer les types d'erreurs de lecture
                // "End of file" = client déconnecté proprement
                if (read_error == asio::error::eof) {
                    std::cout << "Client ID: " << client_id << " déconnecté" << std::endl;
                    registry_.RemoveClient(client_id);
                } else {
                    std::cout << "Erreur lecture header pour client " << client_id << ": " << read_error.message() << std::endl;
                }
            }
        }
    );
}

void NetworkManager::Stop() {
    // 🛑 Vérifier si l'accepteur existe avant de le fermer (éviter crash)
    if (acceptor_)
        acceptor_->close();
    io_.stop();
}

// notif client
void NetworkManager::Update() {
    io_.poll();  // Traiter TOUS les événements ASIO en attente
    PhysicsSystem::Update(world_, 0.016f);  // Update avec deltaTime ~60fps
}

void NetworkManager::SendToClient(int clientId, const Packet& packet) {
    // à implémenter
    std::cout << "Envoi paquet au client " << clientId << std::endl;
}
