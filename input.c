#include "input.h"
#include "settings.h"
#include "ui.h"
#include <math.h>

void HandleInput(EngineState *state, Terrain *terrain) {
  // Regenerate Terrain
  // if (IsKeyPressed(KEY_R)) {
  //     DrawLoadingMessage("Generating Terrain (Please Wait)...");
  //     UnloadTerrain(terrain);
  //     GenerateProceduralTerrain(terrain);
  //     // Clear entities and regenerate them
  //     state->camera_x = gameSettings.mapSize / 2.0f;
  //     state->camera_y = gameSettings.mapSize / 2.0f;
  // }

  // Demo Mode Toggle
  if (IsKeyPressed(KEY_P)) {
      state->demoMode = !state->demoMode;
  }

  if (state->demoMode) {
      state->camera_x -= state->sinphi * 40.0f * state->deltaTime;
      state->camera_y -= state->cosphi * 40.0f * state->deltaTime;
      state->phi += 0.002f * sinf(state->time * 0.25);

      if ((state->camera_x < 0) || (state->camera_x >= gameSettings.mapSize) ||
          (state->camera_y < 0) || (state->camera_y >= gameSettings.mapSize)) {
          state->phi += 180.0f;
      }
  }

  // Edge Scrolling
  Vector2 mousePos = GetMousePosition();
  int edgeSize = 40;
  float moveSpeed = MOVE_SPEED * state->deltaTime;

  if (!state->demoMode) {
    if (mousePos.x < edgeSize) { // Left
      state->camera_x -= state->cosphi * moveSpeed;
      state->camera_y += state->sinphi * moveSpeed;
    }
    if (mousePos.x > GetScreenWidth() - edgeSize) { // Right
      state->camera_x += state->cosphi * moveSpeed;
      state->camera_y -= state->sinphi * moveSpeed;
    }
    if (mousePos.y < edgeSize) { // Top
      state->camera_x -= state->sinphi * moveSpeed;
      state->camera_y -= state->cosphi * moveSpeed;
    }
    if (mousePos.y > GetScreenHeight() - edgeSize) { // Bottom
      state->camera_x += state->sinphi * moveSpeed;
      state->camera_y += state->cosphi * moveSpeed;
    }

    // Mouse Zoom
    // float wheel = GetMouseWheelMove();
    // if (wheel != 0) {
    //   state->camera_z -= wheel * 50.0f;

    //   // Clamp Zoom
    //   if (state->camera_z < 255.0f) state->camera_z = 255.0f;
    //   if (state->camera_z > 3000.0f) state->camera_z = 3000.0f;

    //   if ((state->camera_z > 255.0f) && (state->camera_z < 3000.0f)){
    //     state->camera_x -= state->sinphi * wheel*5.0f;
    //     state->camera_y -= state->cosphi * wheel*5.0f;
    //   }

    //   float t = (state->camera_z - 40.0f) / (3000.0f - 40.0f);
    //   state->horizon = 300.0f * (1.0f - t) + (-1200.0f) * t;
    // }

    // Keyboard Movement
    if (IsKeyDown(KEY_W)) {
      state->camera_x -= state->sinphi * moveSpeed;
      state->camera_y -= state->cosphi * moveSpeed;
    }
    if (IsKeyDown(KEY_S)) {
      state->camera_x += state->sinphi * moveSpeed;
      state->camera_y += state->cosphi * moveSpeed;
    }
    if (IsKeyDown(KEY_A)) {
      state->camera_x -= state->cosphi * moveSpeed;
      state->camera_y += state->sinphi * moveSpeed;
    }
    if (IsKeyDown(KEY_D)) {
      state->camera_x += state->cosphi * moveSpeed;
      state->camera_y -= state->sinphi * moveSpeed;
    }
    if (IsKeyDown(KEY_Q)) {
      state->phi += 1.5f * state->deltaTime;
    }
    if (IsKeyDown(KEY_E)) {
      state->phi -= 1.5f * state->deltaTime;
    }
  }

  // Bounds Checking
  if (state->camera_x < 0) state->camera_x = 0;
  if (state->camera_x >= gameSettings.mapSize) state->camera_x = gameSettings.mapSize - 1;
  if (state->camera_y < 0) state->camera_y = 0;
  if (state->camera_y >= gameSettings.mapSize) state->camera_y = gameSettings.mapSize - 1;

  while (state->phi > 2.0f * PI) state->phi -= 2.0f * PI;
  while (state->phi < 0.0f) state->phi += 2.0f * PI;
}
