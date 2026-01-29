#include "../include/Protocol.hpp"

uint16_t Packet::CalculateChecksum() const {
    uint32_t sum = 0;
    sum += header;
    sum += size;
    sum += static_cast<uint8_t>(type);
    
    // Ajouter des bytes de data
    for (uint8_t byte : data)
        sum += byte;
    
    return sum & 0xFFFF;
}

bool Packet::IsValid() const {
    return CalculateChecksum() == checksum;
}

std::vector<uint8_t> Packet::Serialize() const {
    std::vector<uint8_t> bytes;
    bytes.reserve(6 + data.size()); // header(2) + size(2) + type(1) + checksum(2) + data

    // header
    bytes.push_back(static_cast<uint8_t>(header >> 8));
    bytes.push_back(static_cast<uint8_t>(header & 0xFF));

    // Ajouter la taille
    bytes.push_back(static_cast<uint8_t>(size >> 8));
    bytes.push_back(static_cast<uint8_t>(size & 0xFF));

    // type
    bytes.push_back(static_cast<uint8_t>(type));

    // données
    bytes.insert(bytes.end(), data.begin(), data.end());

    // checksum
    bytes.push_back(static_cast<uint8_t>(checksum >> 8));
    bytes.push_back(static_cast<uint8_t>(checksum & 0xFF));

    return bytes;
}

bool Packet::Deserialize(const std::vector<uint8_t>& bytes, Packet& packet) {
    // minimum header + size + type + checksum
    if (bytes.size() < 7)
        return false;
    
    size_t offset = 0;
    
    // header (2 bytes, big-endian)
    packet.header = (static_cast<uint16_t>(bytes[offset]) << 8) | bytes[offset + 1];
    offset += 2;
    
    // check magic number
    if (packet.header != PACKET_HEADER)
        return false;
    
    // taille (2 bytes)
    packet.size = (static_cast<uint16_t>(bytes[offset]) << 8) | bytes[offset + 1];
    offset += 2;
    
    // type (1 byte)
    packet.type = static_cast<InputType>(bytes[offset]);
    offset += 1;
    
    // données (packet.size bytes)
    packet.data.assign(bytes.begin() + offset, bytes.begin() + offset + packet.size);
    offset += packet.size;
    
    // checksum (2 bytes)
    packet.checksum = (static_cast<uint16_t>(bytes[offset]) << 8) | bytes[offset + 1];
    
    // totale est correcte
    if (bytes.size() != 7 + packet.size)
        return false;
    
    // check checksum
    if (!packet.IsValid())
        return false;
    
    return true;
}