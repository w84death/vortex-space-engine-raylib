#include "ui.h"
#include "settings.h"

void DrawLoadingMessage(const char* text) {
  BeginDrawing();
  ClearBackground(RAYWHITE);
  DrawText(text, 20, 20, 20, DARKGRAY);
  EndDrawing();
}

void DrawGameUI(const EngineState *state) {
    DrawText(TextFormat("FPS: %d", GetFPS()), 10, 10, 20, BLACK);
    DrawText(TextFormat("Pos: %.0f, %.0f; Map size: %dx%d", state->camera_x, state->camera_y, gameSettings.mapSize, gameSettings.mapSize), 10, 35, 20, BLACK);
    DrawText("Mouse or WASD/QE/ZX", 10, 60, 20, BLACK);

    if (state->demoMode) {
        DrawText("DEMO MODE", GetScreenWidth() / 2 - 100, 20, 40, RED);
    }
}
