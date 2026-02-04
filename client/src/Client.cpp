#include "../include/Client.hpp"
#include <iostream>

Client::Client(const std::string& ip, uint16_t port) 
    : ip_(ip), port_(port), socket_(std::make_unique<tcp::socket>(io_)) {
}

Client::~Client() {
    Disconnect();
}

void Client::Connect() {
    try {
        tcp::endpoint endpoint(asio::ip::make_address(ip_), port_);
        socket_->async_connect(endpoint,
            [this](const asio::error_code& error) {
                if (!error) {
                    std::cout << "Connecté au serveur!" << std::endl;
                } else {
                    std::cerr << "Erreur de connexion: " << error.message() << std::endl;
                }
            }
        );
    } catch (std::exception& e) {
        std::cerr << "Erreur Connect: " << e.what() << std::endl;
    }
}

void Client::SendPacket(const Packet& packet) {
    try {
        auto bytes = packet.Serialize();
        asio::async_write(*socket_, asio::buffer(bytes),
            [](const asio::error_code& error, std::size_t /*bytes_transferred*/) {
                if (error) {
                    std::cerr << "Erreur d'envoi: " << error.message() << std::endl;
                }
            }
        );
    } catch (std::exception& e) {
        std::cerr << "Erreur SendPacket: " << e.what() << std::endl;
    }
}

void Client::Update() {
    io_.poll_one();
}

void Client::Disconnect() {
    if (socket_ && socket_->is_open()) {
        socket_->close();
        std::cout << "Déconnecté du serveur." << std::endl;
    }
    io_.stop();
}