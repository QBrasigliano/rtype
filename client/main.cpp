#include "include/Client.hpp"

int main() {
    Client client("127.0.0.1", 8080);
    client.connect();
    
    if (client.isConnected()) {
        // Test envoyer un paquet
        Packet p;
        p.header = PACKET_HEADER;
        p.type = PacketType::MOVE_UP;
        p.size = 0;
        p.data = {};
        
        client.send_packet(p);
    }
    
    return 0;
}
