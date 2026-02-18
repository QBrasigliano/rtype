#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include <string>
#include <cstdint>
#include <vector>

enum class PacketType : uint8_t {
    MOVE_UP = 0x01,         // plus efficace que (PacketType type = PacketType::MOVE_UP;)
    MOVE_DOWN = 0x02,
    MOVE_LEFT = 0x03,
    MOVE_RIGHT = 0x04,
    SHOOT = 0x05,
};

struct Packet {
    uint16_t header;            // 16 bits -> 2 byte 0xDE et 0xAD pour reconnaitre le debut
    uint16_t size;              //  2 bytes selon data
    PacketType type;            // un byte on le vois au dessus ds enum
    std::vector<uint8_t> data;          // size bytes de data
    uint16_t checksum;          // 16 bits pour checker si tt est la

    std::vector<uint8_t> Serialize() const;             // renvoie les header, type, data et checksum en un tableau de byte pour l'envoyer
    void Deserialize(const uint8_t* buffer, size_t length);         // prend un tableau de byte et le transforme en packet
};
