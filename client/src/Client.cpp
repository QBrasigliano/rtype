#include "../include/Client.hpp"
#include <iostream>

Client::Client(const std::string& ip, uint16_t port)
    : ip_(ip), port_(port), socket_(io_), is_connected_(false) {}

Client::~Client() {
    disconnect();
}

void Client::disconnect() {
    if (is_connected_)
        socket_.close();
    is_connected_ = false;
}

bool Client::isConnected() const {
    return is_connected_;
}


void Client::send_packet(const Packet& packet) {
    auto bytes = packet.Serialize();                // serialize et deserialize a faire dans packet de protocol         !!!
    asio::write(socket_, asio::buffer(bytes));
}

Packet Client::receive_packet() {
    Packet packet;
    std::vector<uint8_t> buffer(1024);
    size_t bytes_received = socket_.read_some(asio::buffer(buffer));
    packet.Deserialize(buffer.data(), bytes_received);      // serialize et deserialize a faire dans packet de protocol         !!!
    return packet;
}

void Client::queue_packet(const Packet& packet) {
    packet_queue_.push(packet);
}

void Client::send_queued_packets() {
    while (!packet_queue_.empty()) {
        send_packet(packet_queue_.front());
        packet_queue_.pop();
    }
}

void Client::connect() {
    try {
        // trouver une adresse ip et un port pour se connecter
    } catch (const std::exception& e) {
        // sinonn erruer
    }
}