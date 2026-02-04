#include <raylib.h>
#include "Client.hpp"
#include <iostream>
#include <map>
#include <thread>
#include <chrono>

// envoie pos + input
void SendMovementPacket(Client& client, PacketType moveType, float playerX, float playerY) {
    Packet packet;
    packet.header = PACKET_HEADER;
    packet.size = 4;  // 4 bytes position X(2) + Y(2) - le type n'est PAS dans data !
    packet.type = moveType;
    
    // Encoder position: X(2 bytes) + Y(2 bytes)
    uint16_t x = static_cast<uint16_t>(playerX);
    uint16_t y = static_cast<uint16_t>(playerY);
    
    packet.data = {
        static_cast<uint8_t>((x >> 8) & 0xFF),
        static_cast<uint8_t>(x & 0xFF),
        static_cast<uint8_t>((y >> 8) & 0xFF),
        static_cast<uint8_t>(y & 0xFF)
    };
    
    packet.checksum = packet.CalculateChecksum();
    client.SendPacket(packet);
}

// control
void HandleInput(float& playerX, float& playerY, float playerRadius, float playerSpeed,
                 Client& client) {
    // joueur
    if (IsKeyDown(KEY_UP))
        playerY -= playerSpeed;
    if (IsKeyDown(KEY_DOWN))
        playerY += playerSpeed;
    if (IsKeyDown(KEY_LEFT))
        playerX -= playerSpeed;
    if (IsKeyDown(KEY_RIGHT))
        playerX += playerSpeed;
    
    // terrain
    if (playerX - playerRadius < 0)
        playerX = playerRadius;
    if (playerX + playerRadius > 1024)
        playerX = 1024 - playerRadius;
    if (playerY - playerRadius < 0)
        playerY = playerRadius;
    if (playerY + playerRadius > 768)
        playerY = 768 - playerRadius;
    
    // position - envoyer CHAQUE FRAME (pas toutes les 100ms)
    if (IsKeyDown(KEY_UP))
        SendMovementPacket(client, PacketType::MOVE_UP, playerX, playerY);
    if (IsKeyDown(KEY_DOWN))
        SendMovementPacket(client, PacketType::MOVE_DOWN, playerX, playerY);
    if (IsKeyDown(KEY_LEFT))
        SendMovementPacket(client, PacketType::MOVE_LEFT, playerX, playerY);
    if (IsKeyDown(KEY_RIGHT))
        SendMovementPacket(client, PacketType::MOVE_RIGHT, playerX, playerY);
}

// affichage
void Render(float playerX, float playerY, float playerRadius, std::map<int, std::pair<float, float>>& otherPlayers) {
    BeginDrawing();
    ClearBackground(BLACK);
    
    // Afficher le compteur de joueurs connectés : toi + autres joueurs
    int totalPlayers = 1 + static_cast<int>(otherPlayers.size());
    std::string playerCount = "Joueurs connectés: " + std::to_string(totalPlayers);
    DrawText(playerCount.c_str(), 10, 10, 20, WHITE);
    
    // Afficher les autres joueurs
    for (auto& [id, pos] : otherPlayers) {
        DrawCircle((int)pos.first, (int)pos.second, playerRadius, RED);  // Les autres en ROUGE
    }
    
    // Afficher le joueur local
    DrawCircle((int)playerX, (int)playerY, playerRadius, BLUE);  // Toi en BLEU
    EndDrawing();
}

int main() {
    Client client("127.0.0.1", 4242);
    
    std::cout << "🔌 Connexion au serveur..." << std::endl;
    client.Connect();
    
    // Attendre la connexion avec timeout plus long
    int retries = 0;
    while (!client.IsConnected() && retries < 30) {  // 3 secondes max
        client.Update();
        retries++;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Si pas connecté après timeout, quitter SANS lancer les graphiques
    if (!client.IsConnected()) {
        std::cerr << "❌ IMPOSSIBLE DE SE CONNECTER AU SERVEUR !" << std::endl;
        std::cerr << "📋 Vérifiez que le serveur est bien lancé avec:" << std::endl;
        std::cerr << "   cd build && ./server/rtype_server" << std::endl;
        return 1;
    }
    
    std::cout << "✅ Connecté au serveur! Lancement du jeu..." << std::endl;
    
    // Initialiser la fenêtre SEULEMENT après connexion réussie
    InitWindow(1024, 768, "R-Type");
    SetTargetFPS(60);
    
    float playerX = 100.0f;  // Spawn à gauche
    float playerY = 400.0f;  // Spawn au milieu verticalement
    float playerRadius = 15.0f;
    float playerSpeed = 5.0f;
    
    // Positions des autres joueurs: ID → (X, Y)
    std::map<int, std::pair<float, float>> otherPlayers;
    
    // Callback quand un autre joueur bouge
    client.SetOnPlayerMoved([&](int client_id, float playerX, float playerY) {
        // Utiliser position absolue reçue du serveur
        otherPlayers[client_id] = {playerX, playerY};
    });
    
    // La lecture des messages serveur démarre automatiquement après connexion
    
    while (!WindowShouldClose()) {
        // ASIO events
        client.Update();
        
        // Vérification immédiate de la connexion à chaque frame
        if (!client.IsConnected()) {
            std::cerr << "❌ Connexion perdue avec le serveur! Fermeture du jeu..." << std::endl;
            break;
        }
        
        // control
        HandleInput(playerX, playerY, playerRadius, playerSpeed, client);
        
        // Afficher le rendu
        Render(playerX, playerY, playerRadius, otherPlayers);
    }
    
    client.Disconnect();
    CloseWindow();
    
    return 0;
}
