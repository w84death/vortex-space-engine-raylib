#include "ui.h"
#include "settings.h"
#include "raylib.h"

void DrawLoadingMessage(const char* text) {
  BeginDrawing();
  ClearBackground(THEME_BG);
  
  int textWidth = MeasureText(text, 20);
  int x = GetScreenWidth()/2 - textWidth/2;
  int y = GetScreenHeight()/2 - 10;
  
  DrawText(text, x, y, 20, THEME_ACCENT_LIGHT);
  DrawRectangleLines(x - 20, y - 20, textWidth + 40, 60, THEME_ACCENT);
  DrawText("SYSTEM INITIALIZATION...", x, y + 50, 10, THEME_TEXT_DIM);
  
  EndDrawing();
}

void DrawGameUI(const EngineState *state) {
    // HUD overlay frame
    DrawRectangleLines(10, 10, GetScreenWidth() - 20, GetScreenHeight() - 20, THEME_GRID_LINE);
    
    // Corner accents
    DrawLine(10, 10, 60, 10, THEME_ACCENT);
    DrawLine(10, 10, 10, 60, THEME_ACCENT);
    
    DrawLine(GetScreenWidth() - 60, 10, GetScreenWidth() - 10, 10, THEME_ACCENT);
    DrawLine(GetScreenWidth() - 10, 10, GetScreenWidth() - 10, 60, THEME_ACCENT);
    
    DrawLine(10, GetScreenHeight() - 60, 10, GetScreenHeight() - 10, THEME_ACCENT);
    DrawLine(10, GetScreenHeight() - 10, 60, GetScreenHeight() - 10, THEME_ACCENT);
    
    DrawLine(GetScreenWidth() - 60, GetScreenHeight() - 10, GetScreenWidth() - 10, GetScreenHeight() - 10, THEME_ACCENT);
    DrawLine(GetScreenWidth() - 10, GetScreenHeight() - 60, GetScreenWidth() - 10, GetScreenHeight() - 10, THEME_ACCENT);

    // Top Left Status Panel
    DrawRectangle(15, 15, 260, 80, (Color){THEME_PANEL.r, THEME_PANEL.g, THEME_PANEL.b, 200});
    DrawRectangleLines(15, 15, 260, 80, THEME_ACCENT);
    
    DrawText(TextFormat("FPS: %d", GetFPS()), 25, 25, 20, THEME_ACCENT_LIGHT);
    DrawText(TextFormat("POS: %04.0f, %04.0f", state->camera_x, state->camera_y), 25, 45, 20, THEME_TEXT);
    DrawText(TextFormat("SEC: %dx%d", gameSettings.mapSize, gameSettings.mapSize), 25, 65, 20, THEME_TEXT_DIM);

    // Bottom Left Controls Hint
    DrawText("CTRL: MOUSE | WASD | QE | ZX", 25, GetScreenHeight() - 35, 20, THEME_TEXT_DIM);

    // Demo Mode Indicator
    if (state->demoMode) {
        int w = MeasureText("SIMULATION MODE", 30);
        int x = GetScreenWidth() / 2 - w / 2;
        
        // Blink effect
        if ((int)(GetTime() * 2) % 2 == 0) {
             DrawText("SIMULATION MODE", x, 40, 30, THEME_ACCENT_LIGHT);
        } else {
             DrawText("SIMULATION MODE", x, 40, 30, THEME_ACCENT);
        }
        
        DrawRectangleLines(x - 15, 35, w + 30, 40, THEME_ACCENT);
    }
    
    // Center Crosshair
    int cx = GetScreenWidth() / 2;
    int cy = GetScreenHeight() / 2;
    DrawLine(cx - 15, cy, cx + 15, cy, THEME_ACCENT_LIGHT);
    DrawLine(cx, cy - 15, cx, cy + 15, THEME_ACCENT_LIGHT);
    DrawCircleLines(cx, cy, 8, THEME_ACCENT);
}