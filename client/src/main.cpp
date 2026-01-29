#include <raylib.h>

int main() {
    // Initialiser la fenêtre
    InitWindow(1024, 768, "R-Type");
    
    // FPS
    SetTargetFPS(60);
    
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        EndDrawing();
    }
    
    CloseWindow();
    
    return 0;
}
