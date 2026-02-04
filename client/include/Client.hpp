#pragma once
#include <asio.hpp>
#include <memory>
#include <cstdint>
#include <string>
#include <iostream>
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
    
private:
    asio::io_context io_;
    std::unique_ptr<tcp::socket> socket_;
    std::string ip_;
    uint16_t port_;
};