#include "ui.h"

void DrawLoadingMessage(const char* text) {
  BeginDrawing();
  ClearBackground(RAYWHITE);
  DrawText(text, 20, 20, 20, DARKGRAY);
  EndDrawing();
}

void DrawGameUI(const EngineState *state) {
    DrawText(TextFormat("FPS: %d", GetFPS()), 10, 10, 20, BLACK);
    DrawText(TextFormat("Pos: %.0f, %.0f", state->camera_x, state->camera_y), 10, 35, 20, BLACK);
    DrawText("Strategy View: WASD/Mouse edges to move, Q/E to rotate, Scroll to zoom", 10, 60, 20, DARKBLUE);
}