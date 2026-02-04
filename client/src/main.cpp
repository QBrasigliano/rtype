#include <raylib.h>
#include "Client.hpp"
#include <iostream>

// envoie pos
void SendMovementPacket(Client& client, PacketType moveType) {
    Packet packet;
    packet.header = PACKET_HEADER;
    packet.size = 0;
    packet.type = moveType;
    packet.checksum = packet.CalculateChecksum();
    client.SendPacket(packet);
}

// control
void HandleInput(float& playerX, float& playerY, float playerRadius, float playerSpeed,
                 Client& client, float& sendTimer, float sendInterval) {
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
    
    // position
    sendTimer += GetFrameTime();
    if (sendTimer >= sendInterval) {
        if (IsKeyDown(KEY_UP))
            SendMovementPacket(client, PacketType::MOVE_UP);
        if (IsKeyDown(KEY_DOWN))
            SendMovementPacket(client, PacketType::MOVE_DOWN);
        if (IsKeyDown(KEY_LEFT))
            SendMovementPacket(client, PacketType::MOVE_LEFT);
        if (IsKeyDown(KEY_RIGHT))
            SendMovementPacket(client, PacketType::MOVE_RIGHT);
        sendTimer = 0.0f;
    }
}

// affichage
void Render(float playerX, float playerY, float playerRadius) {
    BeginDrawing();
    ClearBackground(BLACK);
    DrawCircle((int)playerX, (int)playerY, playerRadius, BLUE);
    EndDrawing();
}

int main() {
    // Initialiser la fenêtre
    InitWindow(1024, 768, "R-Type");
    SetTargetFPS(60);
    
    Client client("127.0.0.1", 4242);
    client.Connect();
    
    float playerX = 512.0f;
    float playerY = 700.0f;
    float playerRadius = 15.0f;
    float playerSpeed = 5.0f;
    
    // Timer
    float sendTimer = 0.0f;
    float sendInterval = 0.1f;  // 100ms
    
    while (!WindowShouldClose()) {
        // ASIO events
        client.Update();
        
        // control
        HandleInput(playerX, playerY, playerRadius, playerSpeed, client, sendTimer, sendInterval);
        
        // Afficher le rendu
        Render(playerX, playerY, playerRadius);
    }
    
    client.Disconnect();
    CloseWindow();
    
    return 0;
}
