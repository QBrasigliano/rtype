#pragma once
#include <asio.hpp>
#include <memory>
#include <cstdint>
#include <string>
#include <iostream>
#include <functional>
#include "Protocol.hpp"

using asio::ip::tcp;

class Client {
public:
    Client(const std::string& ip, uint16_t port);
    ~Client();
    
    void Connect();
    void SendPacket(const Packet& packet);
    void Update();
    void Disconnect();
    void ListenForServerMessages();
    bool IsConnected() const { return is_connected_; }
    
    // Callback quand l'autre joueur bouge
    void SetOnPlayerMoved(std::function<void(int, float, float)> callback);         // pas PacketType mais plus ciblé
    
private:
    asio::io_context io_;
    std::shared_ptr<tcp::socket> socket_;
    std::string ip_;
    uint16_t port_;
    bool is_connected_ = false;
    std::function<void(int, float, float)> on_player_moved_;
    
    void ReadServerMessage();
};