#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <asio.hpp>

class Client {
private:
    std::string ip_;
    uint16_t port_;
    asio::io_context io_;          // indispensable, permet de gerer des opérantion sans bloquer le thread principal
    asio::ip::tcp::socket socket_;              // connection tcp pas udp pas envie de gerer les packets perdus
                // tcp est dans l'ip qui est dans asio 
                // et socket est dans tcp

    bool is_connected_;

public:
    Client(const std::string& ip, uint16_t port);
    ~Client();
    
    void connect();
    void disconnect();
    bool isConnected() const;
    void send_packet(const Packet& packet);             // pour l'envoie, j'ai une copie de packet (rapidite) et je l'envoie, pas de bug
    Packet receive_packet();                // la reception renvoie le packet recu
    void send_movement(uint8_t direction);         // envoie une direction a l'aide de la direction sur 8 bit
    void send_ready();
};

#endif