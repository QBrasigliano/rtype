#pragma once
#include <asio.hpp>
#include <memory>
#include <cstdint>
#include <string>
#include <iostream>
#include <functional>
#include "Protocol.hpp"

using asio::ip::tcp;

// bal
struct Bullet {
    float x, y;
    float vx, vy;
    float lifetime;
    int owner_id;
    int bullet_id;
};

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
    void SetOnPlayerMoved(std::function<void(int, float, float)> callback);     // pas PacketType mais plus ciblé
    
    // Callback quand une bullet est créée
    void SetOnBulletSpawned(std::function<void(Bullet)> callback);
    void SetOnEnemySpawned(std::function<void(int, float, float, float, float)> callback);
    
private:
    asio::io_context io_;
    std::shared_ptr<tcp::socket> socket_;
    std::string ip_;
    uint16_t port_;
    bool is_connected_ = false;
    std::function<void(int, float, float)> on_player_moved_;
    std::function<void(Bullet)> on_bullet_spawned_;
    std::function<void(int, float, float, float, float)> on_enemy_spawned_;
    
    void ReadServerMessage();
};