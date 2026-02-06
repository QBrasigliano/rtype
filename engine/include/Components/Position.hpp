#pragma once

#include "../Component.hpp"

class Position : public Component {
public:
    float x;
    float y;

    Position(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}
};
