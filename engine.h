#ifndef ENGINE_H
#define ENGINE_H

#include "raylib.h"
#include "constants.h"

typedef struct {
    float camera_x;
    float camera_y;
    float camera_z;
    float horizon;
    float phi;
    float sinphi;
    float cosphi;
    float deltaTime;
    bool cursorLocked;
    bool demoMode;
} EngineState;

void InitEngine(EngineState *state);
void UpdateEngine(EngineState *state);
void CloseEngine(void);

#endif // ENGINE_H