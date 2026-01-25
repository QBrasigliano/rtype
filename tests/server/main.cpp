#include "../../server/include/Server.hpp"
#include "../../server/include/Protocol.hpp"


int main() {
    // Créer un paquet
    Packet p;
    p.header = PACKET_HEADER;
    p.type = InputType::SHOOT;
    p.data = {0x01, 0x02};
    p.size = 2;
    p.checksum = p.CalculateChecksum();
    
    // Le convertir en bytes
    auto bytes = p.Serialize();
    
    // Le relire depuis les bytes
    Packet p2;
    Packet::Deserialize(bytes, p2);
    
    // Vérifier que c'est identique
    assert(p2.IsValid());
    
    std::cout << "✅ Test OK!" << std::endl;
}