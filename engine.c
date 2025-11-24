#include "engine.h"
#include <math.h>

#include "settings.h"

void InitEngine(EngineState *state) {
  InitWindow(0, 0, "Vertex Space - Huge Terrain");
  ToggleFullscreen();
  SetTargetFPS(60);

  state->camera_x = gameSettings.mapSize / 2.0f;
  state->camera_y = gameSettings.mapSize / 2.0f;
  state->camera_z = 600.0f;
  state->horizon = -150.0f;
  state->phi = 0.785398f;
  state->demoMode = false;
  state->cursorLocked = false;
  state->time = 0.0f;
}

void UpdateEngine(EngineState *state) {
  state->deltaTime = GetFrameTime();
  state->time += state->deltaTime;
  state->sinphi = sinf(state->phi);
  state->cosphi = cosf(state->phi);
}

void CloseEngine(void) {
  CloseWindow();
}
