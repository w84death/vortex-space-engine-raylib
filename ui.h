#ifndef UI_H
#define UI_H

#include "raylib.h"
#include "engine.h"
#include "constants.h"

void DrawLoadingMessage(const char* text);
void DrawGameUI(const EngineState *state);

#endif // UI_H
