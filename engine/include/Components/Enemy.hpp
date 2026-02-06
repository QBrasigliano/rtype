#pragma once

#include "../Component.hpp"

struct EnemyComponent : public Component {
    int enemy_id;
    int health;
    float speed = 100.0f;  // pixels per second
    
    EnemyComponent(int id, int hp = 1)
        : enemy_id(id), health(hp) {}
};
