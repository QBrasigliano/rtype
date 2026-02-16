#pragma once

#include "../World.hpp"
#include "../Components/Position.hpp"
#include "../Components/Velocity.hpp"
#include "../Components/Enemy.hpp"
#include "../Components/Color.hpp"
#include <cmath>
#include <cstdlib>

class EnemySpawner {
public:
    static void SpawnWave(World& world, int& nextEnemyId, int waveNumber, int screenWidth = 800, int screenHeight = 600) {
        int enemiesToSpawn = 2 + waveNumber;  // More enemies each wave

        for (int i = 0; i < enemiesToSpawn; ++i) {
            auto enemy = world.CreateEntity();

            // Random Y position
            float y = (rand() % (screenHeight - 40)) + 20;

            // Spawn from right side, move left
            enemy->AddComponent<Position>(screenWidth + 20.0f, y);
            enemy->AddComponent<Velocity>(-100.0f - (waveNumber * 20), 0.0f);  // Faster each wave
            enemy->AddComponent<EnemyComponent>(nextEnemyId++, 1);  // 1 HP for now
            enemy->AddComponent<ColorComponent>(RGBAColor(0, 255, 0, 255));  // GREEN
        }
    }

    static void SpawnRandomEnemy(World& world, int& nextEnemyId, int screenWidth = 800, int screenHeight = 600) {
        auto enemy = world.CreateEntity();

        float y = (rand() % (screenHeight - 40)) + 20;
        enemy->AddComponent<Position>(screenWidth + 20.0f, y);
        enemy->AddComponent<Velocity>(-120.0f, 0.0f);
        enemy->AddComponent<EnemyComponent>(nextEnemyId++, 1);
        enemy->AddComponent<ColorComponent>(RGBAColor(0, 255, 0, 255));  // GREEN
    }
};
