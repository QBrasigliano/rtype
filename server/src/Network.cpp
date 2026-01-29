#include "../include/Network.hpp"

#include <iostream>

NetworkManager::NetworkManager(uint16_t port) 
    : port_(port) {
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
            if (!error)
                std::cout << "Client connecté!" << std::endl;
            else
                std::cerr << "Erreur connexion: " << error.message() << std::endl;
                        
            AcceptNextClient();
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
