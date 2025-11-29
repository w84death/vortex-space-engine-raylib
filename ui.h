#ifndef UI_H
#define UI_H

#include "raylib.h"
#include "engine.h"

// Military Red Theme
#define THEME_BG            (Color){ 10, 10, 12, 255 }      // Almost Black
#define THEME_PANEL         (Color){ 20, 20, 25, 255 }      // Dark Grey
#define THEME_ACCENT        (Color){ 180, 20, 20, 255 }     // Dark Red
#define THEME_ACCENT_LIGHT  (Color){ 230, 40, 40, 255 }     // Bright Red
#define THEME_TEXT          (Color){ 220, 60, 60, 255 }     // Red Text
#define THEME_TEXT_DIM      (Color){ 120, 40, 40, 255 }     // Dim Red Text
#define THEME_GRID_LINE     (Color){ 60, 20, 20, 255 }      // Grid Lines
#define THEME_GRID_EMPTY    (Color){ 15, 15, 15, 255 }      // Empty Cell
#define THEME_GRID_OOB      (Color){ 5, 5, 5, 255 }         // Out of Bounds

void DrawLoadingMessage(const char* text);
void DrawGameUI(const EngineState *state);

#endif // UI_H