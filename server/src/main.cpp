#include "Network.hpp"
#include "World.hpp"
#include "Systems/EnemySpawner.hpp"
#include "Systems/CollisionSystem.hpp"
#include "Components/Enemy.hpp"
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    NetworkManager manager(4242);
    manager.Start();
    
    std::cout << "Serveur en attente de clients..." << std::endl;
    
    World& world = manager.GetWorld();
    int nextEnemyId = 1;
    auto lastSpawnTime = std::chrono::high_resolution_clock::now();
    auto lastUpdateTime = std::chrono::high_resolution_clock::now();
    
    while (true) {
        auto now = std::chrono::high_resolution_clock::now();
        auto deltaTime = std::chrono::duration<float>(now - lastUpdateTime).count();
        lastUpdateTime = now;
        
        manager.Update();
        
        // Spawn enemies every 2 seconds
        if (std::chrono::duration<float>(now - lastSpawnTime).count() > 2.0f) {
            auto enemy = world.CreateEntity();
            float y = (rand() % 560) + 20;
            enemy->AddComponent<Position>(820.0f, y);
            enemy->AddComponent<Velocity>(-120.0f, 0.0f);
            enemy->AddComponent<EnemyComponent>(nextEnemyId, 1);
            
            // Broadcast ENEMY_SPAWNED to all clients
            Packet enemyPacket;
            enemyPacket.header = PACKET_HEADER;
            enemyPacket.type = PacketType::ENEMY_SPAWNED;
            uint16_t enemy_x = 820;
            uint16_t enemy_y = static_cast<uint16_t>(y);
            uint16_t enemy_vx = 120;
            uint16_t enemy_vy = 0;
            enemyPacket.data = {
                static_cast<uint8_t>(nextEnemyId),
                static_cast<uint8_t>((enemy_x >> 8) & 0xFF),
                static_cast<uint8_t>(enemy_x & 0xFF),
                static_cast<uint8_t>((enemy_y >> 8) & 0xFF),
                static_cast<uint8_t>(enemy_y & 0xFF),
                static_cast<uint8_t>((enemy_vx >> 8) & 0xFF),
                static_cast<uint8_t>(enemy_vx & 0xFF),
                static_cast<uint8_t>((enemy_vy >> 8) & 0xFF),
                static_cast<uint8_t>(enemy_vy & 0xFF)
            };
            enemyPacket.size = 9;
            enemyPacket.checksum = enemyPacket.CalculateChecksum();
            manager.GetRegistry().SendToAll(enemyPacket);
            
            nextEnemyId++;
            lastSpawnTime = now;
        }
        
        // Sleep to not consume 100% CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    
    manager.Stop();
    return 0;
}
