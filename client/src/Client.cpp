#include "../include/Client.hpp"
#include <iostream>

Client::Client(const std::string& ip, uint16_t port) 
    : ip_(ip), port_(port), socket_(std::make_shared<tcp::socket>(io_)) {
}

Client::~Client() {
    Disconnect();
}

void Client::Connect() {
    try {
        tcp::endpoint endpoint(asio::ip::make_address(ip_), port_);
        socket_->async_connect(endpoint,
            [this](const asio::error_code& error) {
                // ✅ Vérifier si la connexion au serveur s'est bien passée
                if (!error) {
                    is_connected_ = true;
                    std::cout << "Connecté au serveur!" << std::endl;
                    // lecture des messages serveur
                    ReadServerMessage();
                } else {
                    is_connected_ = false;
                    std::cerr << "Erreur de connexion: " << error.message() << std::endl;
                }
            }
        );
    } catch (std::exception& e) {
        is_connected_ = false;
        std::cerr << "Erreur Connect: " << e.what() << std::endl;
    }
}

void Client::SendPacket(const Packet& packet) {
    try {
        // 🔌 Vérifier si on est bien connecté avant d'envoyer un packet
        if (!is_connected_) {
            std::cerr << "⚠️ Non connecté au serveur! Vérifiez que le serveur est lancé." << std::endl;
            return;
        }
        
        auto bytes = packet.Serialize();
        asio::async_write(*socket_, asio::buffer(bytes),
            [this](const asio::error_code& error, std::size_t /*bytes_transferred*/) {
                // ⚠️ Vérifier s'il y a eu une erreur lors de l'envoi du packet
                if (error) {
                    // 🔌 Vérifier si on était encore marqué comme connecté (pour éviter spam d'erreurs)
                    if (is_connected_) {
                        std::cerr << "Serveur déconnecté!" << error.message() << std::endl;
                        is_connected_ = false;
                    }
                }
            }
        );
    } catch (std::exception& e) {
        std::cerr << "Erreur SendPacket: " << e.what() << std::endl;
    }
}

void Client::Update() {
    try {
        io_.poll();  // Traiter TOUS les événements ASIO en attente
    } catch (std::exception& e) {
        std::cerr << "Erreur Update: " << e.what() << std::endl;
    }
}

void Client::Disconnect() {
    if (socket_ && socket_->is_open()) {
        socket_->close();
        std::cout << "Déconnecté du serveur." << std::endl;
    }
    io_.stop();
}

void Client::SetOnPlayerMoved(std::function<void(int, float, float)> callback) {
    on_player_moved_ = callback;
}

void Client::ListenForServerMessages() {
    ReadServerMessage();
}

void Client::ReadServerMessage() {
    // buffer entête (4 bytes)
    auto header_buffer = std::make_shared<std::vector<uint8_t>>(4);
    
    // lire entête (asynchrone avec async_read)
    asio::async_read(*socket_, asio::buffer(*header_buffer),
        [this, header_buffer](const asio::error_code& error, std::size_t /*bytes*/) mutable {
            // 📖 Vérifier si la lecture de l'en-tête du packet serveur s'est bien passée
            if (!error) {
                // extraire header et taille payload
                uint16_t header = (static_cast<uint16_t>((*header_buffer)[0]) << 8) | (*header_buffer)[1];
                // 🔒 Vérifier que le packet serveur commence bien par 0xDEAD
                if (header != PACKET_HEADER) {
                    ReadServerMessage();
                    return;
                }
                uint16_t payload_size = (static_cast<uint16_t>((*header_buffer)[2]) << 8) | (*header_buffer)[3];

                // buffer payload
                auto payload_buffer = std::make_shared<std::vector<uint8_t>>(1 + payload_size + 2);
                
                //lire buf payload
                asio::async_read(*socket_, asio::buffer(*payload_buffer),
                    [this, header_buffer, payload_buffer](const asio::error_code& payload_error, std::size_t /*bytes*/) mutable {
                        // 📦 Vérifier si la lecture du contenu du packet serveur s'est bien passée
                        if (!payload_error) {
                            // reconstruire paquet complet
                            std::vector<uint8_t> complete_packet;
                            complete_packet.insert(complete_packet.end(), header_buffer->begin(), header_buffer->end());
                            complete_packet.insert(complete_packet.end(), payload_buffer->begin(), payload_buffer->end());
                            
                            Packet packet;
                            // ✅ Vérifier si le packet reçu du serveur est valide (checksum + structure)
                            if (Packet::Deserialize(complete_packet, packet)) {
                                // if PLAYER_MOVED - format: client_id(1) + X(2) + Y(2) = 5 bytes
                                // 🏃 Vérifier si c'est un mouvement d'un autre joueur ET qu'il contient les données
                                if (packet.type == PacketType::PLAYER_MOVED && packet.data.size() >= 5) {
                                    int client_id = packet.data[0];
                                    uint16_t playerX = (static_cast<uint16_t>(packet.data[1]) << 8) | packet.data[2];
                                    uint16_t playerY = (static_cast<uint16_t>(packet.data[3]) << 8) | packet.data[4];
                                    
                                    // Appeler le callback -> position absolue
                                    // 🗒 Vérifier si un callback a été défini pour les mouvements de joueurs
                                    if (on_player_moved_)
                                        on_player_moved_(client_id, (float)playerX, (float)playerY);
                                }
                                // if BULLET_SPAWNED - format: bullet_id(1) + x(2) + y(2) + vx(2) + vy(2) = 9 bytes
                                // 🔥 Vérifier si c'est un spawn de bullet ET qu'il contient toutes les données
                                else if (packet.type == PacketType::BULLET_SPAWNED && packet.data.size() >= 9) {
                                    Bullet bullet;
                                    bullet.bullet_id = packet.data[0];
                                    
                                    uint16_t x = (static_cast<uint16_t>(packet.data[1]) << 8) | packet.data[2];
                                    uint16_t y = (static_cast<uint16_t>(packet.data[3]) << 8) | packet.data[4];
                                    uint16_t vx = (static_cast<uint16_t>(packet.data[5]) << 8) | packet.data[6];
                                    uint16_t vy = (static_cast<uint16_t>(packet.data[7]) << 8) | packet.data[8];
                                    
                                    bullet.x = static_cast<float>(x);
                                    bullet.y = static_cast<float>(y);
                                    bullet.vx = static_cast<float>(vx);
                                    bullet.vy = static_cast<float>(vy) - 32768;
                                    bullet.lifetime = 3.0f;
                                    bullet.owner_id = 0;
                                    
                                    // 🗒 Vérifier si un callback a été défini pour les spawns de bullets
                                    if (on_bullet_spawned_)
                                        on_bullet_spawned_(bullet);
                                }
                            }
                            ReadServerMessage();
                        } else {
                            // Erreur de lecture = serveur déconnecté
                            std::cout << "Déconnecté du serveur." << std::endl;
                            is_connected_ = false;
                        }
                    }
                );
            } else {
                // Erreur de lecture header = serveur déconnecté
                std::cout << "Déconnecté du serveur." << std::endl;
                is_connected_ = false;
            }
        }
    );
}

void Client::SetOnBulletSpawned(std::function<void(Bullet)> callback) {
    on_bullet_spawned_ = callback;
}