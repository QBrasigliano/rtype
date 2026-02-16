#pragma once

#include "../World.hpp"
#include "../Components/Position.hpp"
#include "../Components/Enemy.hpp"
#include "../Components/Bullet.hpp"
#include <vector>
#include <cmath>

class CollisionSystem {
public:
    // Returns list of destroyed enemy IDs
    static std::vector<int> CheckCollisions(World& world, float collisionRadius = 15.0f) {
        std::vector<int> destroyedEnemies;
        std::vector<uint32_t> entitiesToRemove;
        
        const auto& entities = world.GetEntities();
        
        // Collect bullets and enemies first
        std::vector<std::shared_ptr<Entity>> bullets, enemies;
        for (const auto& entity : entities) {
            if (entity->HasComponent<BulletComponent>()) {
                bullets.push_back(entity);
            }
            if (entity->HasComponent<EnemyComponent>()) {
                enemies.push_back(entity);
            }
        }
        
        // Check collisions
        for (const auto& bullet : bullets) {
            if (!bullet->HasComponent<Position>()) continue;
            Position& bulletPos = bullet->GetComponent<Position>();
            
            for (const auto& enemy : enemies) {
                if (!enemy->HasComponent<Position>()) continue;
                Position& enemyPos = enemy->GetComponent<Position>();
                
                // Calculate distance
                float dx = bulletPos.x - enemyPos.x;
                float dy = bulletPos.y - enemyPos.y;
                float distance = std::sqrt(dx * dx + dy * dy);
                
                // Collision detected
                if (distance < collisionRadius) {
                    entitiesToRemove.push_back(bullet->GetId());
                    
                    // Damage enemy
                    EnemyComponent& enemyComp = enemy->GetComponent<EnemyComponent>();
                    enemyComp.health--;
                    
                    if (enemyComp.health <= 0) {
                        destroyedEnemies.push_back(enemyComp.enemy_id);
                        entitiesToRemove.push_back(enemy->GetId());
                    }
                    
                    break;  // Bullet can only hit one enemy
                }
            }
        }
        
        // Remove marked entities AFTER iteration
        for (uint32_t id : entitiesToRemove) {
            world.RemoveEntity(id);
        }
        
        return destroyedEnemies;
    }
};
