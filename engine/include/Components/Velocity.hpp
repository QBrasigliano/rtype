#pragma once

#include "../Component.hpp"

class Velocity : public Component {
public:
    float vx;
    float vy;

    Velocity(float vx = 0.0f, float vy = 0.0f) : vx(vx), vy(vy) {}
};
