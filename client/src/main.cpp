#include <raylib.h>
#include "Client.hpp"
#include "World.hpp"
#include "Components/Position.hpp"
#include "Components/Velocity.hpp"
#include "Components/Color.hpp"
#include "Systems/RenderSystem.hpp"
#include "Systems/PhysicsSystem.hpp"
#include <iostream>
#include <map>
#include <thread>
#include <chrono>
#include <vector>
#include <algorithm>

// Component pour identifier le joueur local
class LocalPlayerComponent : public Component {};

// Component pour identifier les autres joueurs
class RemotePlayerComponent : public Component {
public:
    int client_id;
    RemotePlayerComponent(int id) : client_id(id) {}
};

// Component pour les balles
class BulletComponent : public Component {
public:
    int bullet_id;
    float lifetime;
    BulletComponent(int id, float lt) : bullet_id(id), lifetime(lt) {}
};

// Component pour les ennemis
class LocalEnemyComponent : public Component {
public:
    int enemy_id;
    LocalEnemyComponent(int id) : enemy_id(id) {}
};

// Component pour les limites d'écran
class ScreenBoundsComponent : public Component {
public:
    float radius;
    ScreenBoundsComponent(float r) : radius(r) {}
};

void SendMovementPacket(Client& client, PacketType moveType, float playerX, float playerY) {
    Packet packet;
    packet.header = PACKET_HEADER;
    packet.size = 4;
    packet.type = moveType;
    
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

void SendShootPacket(Client& client, float playerX, float playerY) {
    Packet packet;
    packet.header = PACKET_HEADER;
    packet.size = 4;
    packet.type = PacketType::SHOOT;
    
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

void HandleInput(World& world, Client& client) {
    // Trouver le joueur local
    auto localPlayer = world.GetEntity(0); // Le joueur local a ID 0
    if (!localPlayer || !localPlayer->HasComponent<Position>())
        return;
    
    Position& playerPos = localPlayer->GetComponent<Position>();
    ScreenBoundsComponent& bounds = localPlayer->GetComponent<ScreenBoundsComponent>();
    float playerSpeed = 5.0f;
    
    // Déplacement
    if (IsKeyDown(KEY_UP))
        playerPos.y -= playerSpeed;
    if (IsKeyDown(KEY_DOWN))
        playerPos.y += playerSpeed;
    if (IsKeyDown(KEY_LEFT))
        playerPos.x -= playerSpeed;
    if (IsKeyDown(KEY_RIGHT))
        playerPos.x += playerSpeed;
    
    // Limites écran
    if (playerPos.x - bounds.radius < 0)
        playerPos.x = bounds.radius;
    if (playerPos.x + bounds.radius > 1024)
        playerPos.x = 1024 - bounds.radius;
    if (playerPos.y - bounds.radius < 0)
        playerPos.y = bounds.radius;
    if (playerPos.y + bounds.radius > 768)
        playerPos.y = 768 - bounds.radius;
    
    // Envoyer position au serveur
    if (IsKeyDown(KEY_UP))
        SendMovementPacket(client, PacketType::MOVE_UP, playerPos.x, playerPos.y);
    if (IsKeyDown(KEY_DOWN))
        SendMovementPacket(client, PacketType::MOVE_DOWN, playerPos.x, playerPos.y);
    if (IsKeyDown(KEY_LEFT))
        SendMovementPacket(client, PacketType::MOVE_LEFT, playerPos.x, playerPos.y);
    if (IsKeyDown(KEY_RIGHT))
        SendMovementPacket(client, PacketType::MOVE_RIGHT, playerPos.x, playerPos.y);
    
    // Tir
    if (IsKeyPressed(KEY_SPACE))
        SendShootPacket(client, playerPos.x, playerPos.y);
}

void RenderUI(const World& world) {
    // Compter les joueurs
    int playerCount = 0;
    for (const auto& entity : world.GetEntities()) {
        if (entity->HasComponent<LocalPlayerComponent>() || entity->HasComponent<RemotePlayerComponent>())
            playerCount++;
    }
    
    std::string playerCountStr = "Joueurs connectés: " + std::to_string(playerCount);
    DrawText(playerCountStr.c_str(), 10, 10, 20, WHITE);
}

void UpdateBulletsLifetime(World& world, float deltaTime) {
    std::vector<uint32_t> bulletsToRemove;
    
    for (const auto& entity : world.GetEntities()) {
        if (entity->HasComponent<BulletComponent>()) {
            BulletComponent& bullet = entity->GetComponent<BulletComponent>();
            bullet.lifetime -= deltaTime;
            
            // Vérifier positions hors écran
            if (entity->HasComponent<Position>()) {
                Position& pos = entity->GetComponent<Position>();
                if (pos.x > 1024 || pos.x < 0 || pos.y > 768 || pos.y < 0 || bullet.lifetime <= 0)
                    bulletsToRemove.push_back(entity->GetId());
            }
        }
    }
    
    for (uint32_t id : bulletsToRemove)
        world.RemoveEntity(id);
}

int main() {
    Client client("127.0.0.1", 4242);
    
    std::cout << "Connexion au serveur..." << std::endl;
    client.Connect();
    
    int retries = 0;
    while (!client.IsConnected() && retries < 10) {
        client.Update();
        retries++;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    if (!client.IsConnected()) {
        std::cerr << "IMPOSSIBLE DE SE CONNECTER AU SERVEUR !" << std::endl;
        return 1;
    }
    
    std::cout << "Connecté au serveur! Lancement du jeu..." << std::endl;
    
    InitWindow(1024, 768, "R-Type");
    SetTargetFPS(60);
    
    // Créer la World ECS
    World world;
    
    // Créer le joueur local dans l'ECS
    auto localPlayer = world.CreateEntity();
    localPlayer->AddComponent<LocalPlayerComponent>();
    localPlayer->AddComponent<Position>(100.0f, 400.0f);
    localPlayer->AddComponent<Velocity>(0.0f, 0.0f);
    localPlayer->AddComponent<ScreenBoundsComponent>(15.0f);
    localPlayer->AddComponent<ColorComponent>(RGBAColor(0, 0, 255, 255));  // BLUE
    
    // Callbacks pour les messages du serveur
    client.SetOnPlayerMoved([&](int clientId, float px, float py) {
        // Chercher ou créer l'entity pour ce client distant
        auto remotePlayer = world.GetEntity(clientId + 1000); // ID offset pour éviter collision avec ID local
        if (!remotePlayer) {
            remotePlayer = world.CreateEntity();
            remotePlayer->AddComponent<RemotePlayerComponent>(clientId);
            remotePlayer->AddComponent<Position>(px, py);
            remotePlayer->AddComponent<ColorComponent>(RGBAColor(255, 0, 0, 255));  // RED
        } else {
            remotePlayer->GetComponent<Position>().x = px;
            remotePlayer->GetComponent<Position>().y = py;
        }
    });
    
    client.SetOnBulletSpawned([&](Bullet bulletData) {
        auto bulletEntity = world.CreateEntity();
        bulletEntity->AddComponent<BulletComponent>(bulletData.bullet_id, bulletData.lifetime);
        bulletEntity->AddComponent<Position>(bulletData.x, bulletData.y);
        bulletEntity->AddComponent<Velocity>(bulletData.vx, bulletData.vy);
        bulletEntity->AddComponent<ColorComponent>(RGBAColor(255, 255, 0, 255));  // YELLOW
    });
    
    client.SetOnEnemySpawned([&](int enemyId, float x, float y, float vx, float vy) {
        auto enemyEntity = world.CreateEntity();
        enemyEntity->AddComponent<LocalEnemyComponent>(enemyId);
        enemyEntity->AddComponent<Position>(x, y);
        enemyEntity->AddComponent<Velocity>(vx, vy);
        enemyEntity->AddComponent<ColorComponent>(RGBAColor(0, 255, 0, 255));  // GREEN
    });
    
    while (!WindowShouldClose()) {
        client.Update();
        
        if (!client.IsConnected()) {
            std::cerr << "Connexion perdue!" << std::endl;
            break;
        }
        
        float deltaTime = GetFrameTime();
        
        HandleInput(world, client);
        PhysicsSystem::Update(world, deltaTime);
        UpdateBulletsLifetime(world, deltaTime);
        
        BeginDrawing();
        ClearBackground(BLACK);
        RenderSystem::Render(world);
        RenderUI(world);
        EndDrawing();
    }
    
    client.Disconnect();
    CloseWindow();
    
    return 0;
}

