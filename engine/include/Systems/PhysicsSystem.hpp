#pragma once

#include "../World.hpp"
#include "../Components/Position.hpp"
#include "../Components/Velocity.hpp"

class PhysicsSystem {
public:
    static void Update(World& world, float deltaTime) {
        for (const auto& entity : world.GetEntities()) {
            if (entity->HasComponent<Position>() && entity->HasComponent<Velocity>()) {
                Position& pos = entity->GetComponent<Position>();
                Velocity& vel = entity->GetComponent<Velocity>();
                
                // Update position based on velocity
                pos.x += vel.vx * deltaTime;
                pos.y += vel.vy * deltaTime;
            }
        }
    }
};
