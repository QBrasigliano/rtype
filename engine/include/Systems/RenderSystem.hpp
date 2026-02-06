#pragma once

#include "../World.hpp"
#include "../Components/Position.hpp"
#include "../Components/Color.hpp"
#include <raylib.h>

class RenderSystem {
public:
    static void Render(const World& world) {
        for (const auto& entity : world.GetEntities()) {
            if (entity->HasComponent<Position>()) {
                Position& pos = entity->GetComponent<Position>();
                
                // Get color if available, otherwise use default BLUE
                RGBAColor color(0, 0, 255, 255);  // default BLUE
                if (entity->HasComponent<ColorComponent>()) {
                    color = entity->GetComponent<ColorComponent>().color;
                }
                
                // Convert to raylib Color
                ::Color raycolor = { color.r, color.g, color.b, color.a };
                
                // Draw circle
                DrawCircle((int)pos.x, (int)pos.y, 10.0f, raycolor);
            }
        }
    }
};
