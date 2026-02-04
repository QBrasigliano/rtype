#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include <cstdint>
#include <vector>
#include <string>
#include <iostream>
#include <cassert>
#include <iostream>


constexpr uint16_t PACKET_HEADER = 0xDEAD;

enum class InputType : uint8_t {
    MOVE_UP = 0x01,
    MOVE_DOWN = 0x02,
    MOVE_LEFT = 0x03,
    MOVE_RIGHT = 0x04,
    SHOOT = 0x05,
};

struct Packet {
    uint16_t header;
    uint16_t size;
    InputType type;
    std::vector<uint8_t> data;
    uint16_t checksum;

    uint16_t CalculateChecksum() const;

    bool IsValid() const;
    std::vector<uint8_t> Serialize() const;
    static bool Deserialize(const std::vector<uint8_t>& bytes, Packet& packet);
};

#endif
