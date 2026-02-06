#pragma once

#include "../Component.hpp"

struct BulletComponent : public Component {
    int bullet_id;
    int owner_id;
    float lifetime;
    float elapsed_time = 0.0f;
    
    BulletComponent(int id, int owner, float lt) 
        : bullet_id(id), owner_id(owner), lifetime(lt) {}
};
