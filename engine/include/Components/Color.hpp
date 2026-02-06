#pragma once

#include "../Component.hpp"

struct RGBAColor {
    uint8_t r, g, b, a;
    RGBAColor(uint8_t red = 255, uint8_t green = 255, uint8_t blue = 255, uint8_t alpha = 255)
        : r(red), g(green), b(blue), a(alpha) {}
};

struct ColorComponent : public Component {
    RGBAColor color;
    
    ColorComponent(RGBAColor c = RGBAColor())
        : color(c) {}
};
