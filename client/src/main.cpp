#include <raylib.h>
#include "Client.hpp"
#include <iostream>
#include <map>
#include <thread>
#include <chrono>
#include <vector>
#include <algorithm>

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

// tir
void SendShootPacket(Client& client, float playerX, float playerY) {
    Packet packet;
    packet.header = PACKET_HEADER;
    packet.size = 4;  // 4 bytes position X(2) + Y(2) 
    packet.type = PacketType::SHOOT;
    
    // Encoder position du tir: X(2 bytes) + Y(2 bytes)
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
    // ⬆️ Vérifier si la flèche HAUT est pressée pour monter
    if (IsKeyDown(KEY_UP))
        playerY -= playerSpeed;
    // ⬇️ Vérifier si la flèche BAS est pressée pour descendre
    if (IsKeyDown(KEY_DOWN))
        playerY += playerSpeed;
    // ⬅️ Vérifier si la flèche GAUCHE est pressée pour aller à gauche
    if (IsKeyDown(KEY_LEFT))
        playerX -= playerSpeed;
    // ➡️ Vérifier si la flèche DROITE est pressée pour aller à droite
    if (IsKeyDown(KEY_RIGHT))
        playerX += playerSpeed;
    
    // terrain
    // 🛡️ Vérifier si le joueur sort par la gauche de l'écran
    if (playerX - playerRadius < 0)
        playerX = playerRadius;
    // 🛡️ Vérifier si le joueur sort par la droite de l'écran (1024px de large)
    if (playerX + playerRadius > 1024)
        playerX = 1024 - playerRadius;
    // 🛡️ Vérifier si le joueur sort par le haut de l'écran
    if (playerY - playerRadius < 0)
        playerY = playerRadius;
    // 🛡️ Vérifier si le joueur sort par le bas de l'écran (768px de haut)
    if (playerY + playerRadius > 768)
        playerY = 768 - playerRadius;
    
    // position - envoyer CHAQUE FRAME (pas toutes les 100ms)
    // 📤 Envoyer position au serveur si flèche HAUT pressée
    if (IsKeyDown(KEY_UP))
        SendMovementPacket(client, PacketType::MOVE_UP, playerX, playerY);
    // 📤 Envoyer position au serveur si flèche BAS pressée
    if (IsKeyDown(KEY_DOWN))
        SendMovementPacket(client, PacketType::MOVE_DOWN, playerX, playerY);
    // 📤 Envoyer position au serveur si flèche GAUCHE pressée
    if (IsKeyDown(KEY_LEFT))
        SendMovementPacket(client, PacketType::MOVE_LEFT, playerX, playerY);
    // 📤 Envoyer position au serveur si flèche DROITE pressée
    if (IsKeyDown(KEY_RIGHT))
        SendMovementPacket(client, PacketType::MOVE_RIGHT, playerX, playerY);
    
    // tir
    // 🔥 Vérifier si la barre ESPACE est pressée pour tirer (IsKeyPressed = une seule fois par appui)
    if (IsKeyPressed(KEY_SPACE))
        SendShootPacket(client, playerX, playerY);
}

// affichage
void Render(float playerX, float playerY, float playerRadius, std::map<int, std::pair<float, float>>& otherPlayers, std::vector<Bullet>& bullets) {
    BeginDrawing();
    ClearBackground(BLACK);
    
    // Afficher le compteur de joueurs connectés : toi + autres joueurs
    int totalPlayers = 1 + static_cast<int>(otherPlayers.size());
    std::string playerCount = "Joueurs connectés: " + std::to_string(totalPlayers);
    DrawText(playerCount.c_str(), 10, 10, 20, WHITE);
    
    // Afficher les bullets
    for (auto& bullet : bullets)
        DrawCircle((int)bullet.x, (int)bullet.y, 3, YELLOW);
    
    // Afficher les autres joueurs
    for (auto& [id, pos] : otherPlayers)
        DrawCircle((int)pos.first, (int)pos.second, playerRadius, RED);
    
    // Afficher le joueur local
    DrawCircle((int)playerX, (int)playerY, playerRadius, BLUE);
    EndDrawing();
}

// mise à jour des bullets
void UpdateBullets(std::vector<Bullet>& bullets, float deltaTime) {
    // pos bullet
    for (auto& bullet : bullets) {
        bullet.x += bullet.vx * deltaTime;
        bullet.y += bullet.vy * deltaTime;
        bullet.lifetime -= deltaTime;
    }
    
    // hors ecran
    bullets.erase(
        std::remove_if(bullets.begin(), bullets.end(), [](const Bullet& bullet) {
            return bullet.x > 1024 || bullet.x < 0 || 
                   bullet.y > 768 || bullet.y < 0 || 
                   bullet.lifetime <= 0;
        }),
        bullets.end()
    );
}

int main() {
    Client client("127.0.0.1", 4242);
    
    std::cout << "Connexion au serveur..." << std::endl;
    client.Connect();
    
    // timout de co 
    int retries = 0;
    // ⏳ Boucler tant qu'on n'est PAS connecté ET qu'on n'a pas atteint le timeout
    while (!client.IsConnected() && retries < 10) {  // 1 secondes max
        client.Update();
        retries++;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // si pas co, quitter 
    // ❌ Vérifier si on n'a toujours pas réussi à se connecter après le timeout
    if (!client.IsConnected()) {
        std::cerr << "IMPOSSIBLE DE SE CONNECTER AU SERVEUR !" << std::endl;
        std::cerr << "Vérifiez que le serveur est bien lancé avec:" << std::endl;
        std::cerr << "   cd build && ./server/rtype_server" << std::endl;
        return 1;
    }
    
    std::cout << "Connecté au serveur! Lancement du jeu..." << std::endl;
    
    // Initialiser la fenêtre SEULEMENT après connexion réussie
    InitWindow(1024, 768, "R-Type");
    SetTargetFPS(60);
    
    float playerX = 100.0f;  // Spawn à gauche
    float playerY = 400.0f;  // Spawn au milieu verticalement
    float playerRadius = 15.0f;
    float playerSpeed = 5.0f;
    
    // Positions des autres joueurs: ID → (X, Y)
    std::map<int, std::pair<float, float>> otherPlayers;
    
    std::vector<Bullet> bullets;
    
    // Callback quand un autre joueur bouge
    client.SetOnPlayerMoved([&](int client_id, float playerX, float playerY) {
        // poiton absolue
        otherPlayers[client_id] = {playerX, playerY};
    });
    
    // Callback pour les bullets
    client.SetOnBulletSpawned([&](Bullet bullet) { 
        bullets.push_back(bullet); 
        std::cout << "Bullet reçue: ID=" << bullet.bullet_id 
                  << " pos=(" << bullet.x << "," << bullet.y << ")" << std::endl;
    });
    
    // La lecture des messages serveur démarre automatiquement après connexion
    
    // 🎮 Boucle principale du jeu - continue tant que la fenêtre n'est PAS fermée
    while (!WindowShouldClose()) {
        // ASIO events
        client.Update();
        
        // Vérification immédiate de la connexion à chaque frame
        // 🔌 Vérifier si on a perdu la connexion avec le serveur
        if (!client.IsConnected()) {
            std::cerr << "Connexion perdue avec le serveur! Fermeture du jeu..." << std::endl;
            break;
        }
        
        float deltaTime = GetFrameTime(); // Raylib fonction pour deltaTime
        
        // control
        HandleInput(playerX, playerY, playerRadius, playerSpeed, client);
        
        // MAj mouvement balles
        UpdateBullets(bullets, deltaTime);
        
        // Afficher le rendu
        Render(playerX, playerY, playerRadius, otherPlayers, bullets);
    }
    
    client.Disconnect();
    CloseWindow();
    
    return 0;
}
