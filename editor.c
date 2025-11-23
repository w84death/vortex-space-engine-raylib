#include "editor.h"
#include "raylib.h"
#include "constants.h"
#include "entities.h"
#include "terrain.h"
#include "renderer.h"
#include "engine.h"
#include "settings.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define EDITOR_GRID_SIZE MAX_ENTITY_SIZE
#define CELL_SIZE 24
#define GRID_OFFSET_X 20
#define GRID_OFFSET_Y 20

// Preview Terrain Size
#define PREVIEW_MAP_SIZE 512

static const Color PALETTE[32] = {
    DB_BLACK, DB_VALHALLA, DB_LOULOU, DB_OILED_CEDAR, DB_ROPE, DB_TAHITI_GOLD, DB_TWINE, DB_PANCHO,
    DB_GOLDEN_FIZZ, DB_ATLANTIS, DB_CHRISTI, DB_ELF_GREEN, DB_DELL, DB_VERDUN_GREEN, DB_OPAL, DB_DEEP_KOAMARU,
    DB_VENICE_BLUE, DB_ROYAL_BLUE, DB_KURT, DB_WIND_BLUE, DB_LINK_WATER, DB_WHITE, DB_SILVER, DB_IRON,
    DB_SHUTTLE_GREY, DB_CASCADE, DB_MING, DB_MOZART, DB_OLD_ROSE, DB_MAUVELOUS, DB_APPLE_BLOSSOM, DB_SAPLING
};

static VoxelModel currentModel;
static Color selectedColor = DB_MOZART;
static int selectedHeight = 10;

// Forward declaration
bool GuiButton(Rectangle bounds, const char* text);
float GuiSlider(Rectangle bounds, const char* text, float value, float min, float max);

void InitEditorModel() {
    currentModel.width = 16;
    currentModel.length = 16;

    // Clear
    for(int i=0; i<MAX_ENTITY_SIZE*MAX_ENTITY_SIZE; i++) {
        currentModel.heights[i] = 0;
        currentModel.colors[i] = BLANK;
    }
}

void DrawEditorGrid() {
    // Draw Background
    DrawRectangle(GRID_OFFSET_X - 2, GRID_OFFSET_Y - 2,
                  EDITOR_GRID_SIZE * CELL_SIZE + 4, EDITOR_GRID_SIZE * CELL_SIZE + 4, DARKGRAY);

    for (int y = 0; y < EDITOR_GRID_SIZE; y++) {
        for (int x = 0; x < EDITOR_GRID_SIZE; x++) {
            int drawX = GRID_OFFSET_X + x * CELL_SIZE;
            int drawY = GRID_OFFSET_Y + y * CELL_SIZE;

            Color c = WHITE; // Empty background
            if (x >= currentModel.width || y >= currentModel.length) {
                c = LIGHTGRAY; // Out of bounds
            } else {
                int index = y * MAX_ENTITY_SIZE + x;
                if (currentModel.heights[index] > 0) {
                    c = currentModel.colors[index];
                }
            }

            DrawRectangle(drawX, drawY, CELL_SIZE-1, CELL_SIZE-1, c);
        }
    }

    // Draw Border for current bounds
    DrawRectangleLines(GRID_OFFSET_X, GRID_OFFSET_Y, currentModel.width * CELL_SIZE, currentModel.length * CELL_SIZE, RED);
}

void HandleGridInput() {
    Vector2 mouse = GetMousePosition();

    // Grid interaction
    int gx = (mouse.x - GRID_OFFSET_X) / CELL_SIZE;
    int gy = (mouse.y - GRID_OFFSET_Y) / CELL_SIZE;

    // Safety check for grid bounds
    if (gx < 0 || gy < 0 || gx >= EDITOR_GRID_SIZE || gy >= EDITOR_GRID_SIZE) return;

    bool insideModel = (gx < currentModel.width && gy < currentModel.length);

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && insideModel) {
        int index = gy * MAX_ENTITY_SIZE + gx;
        currentModel.heights[index] = (unsigned char)selectedHeight;
        currentModel.colors[index] = selectedColor;
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && insideModel) {
        int index = gy * MAX_ENTITY_SIZE + gx;
        currentModel.heights[index] = 0;
        currentModel.colors[index] = BLANK;
    }
}

void UpdatePreviewTerrain(Terrain *t) {
    // Clear to Water
    for(int i=0; i<PREVIEW_MAP_SIZE*PREVIEW_MAP_SIZE; i++) {
        t->heightmapRaw[i] = LEVEL_WATER;
        t->colormapData[i] = DB_VENICE_BLUE; // Water
    }

    // Paint Model in center
    int cx = PREVIEW_MAP_SIZE / 2 - currentModel.width / 2;
    int cy = PREVIEW_MAP_SIZE / 2 - currentModel.length / 2;

    for(int y=0; y<currentModel.length; y++) {
        for(int x=0; x<currentModel.width; x++) {
            int modelIndex = y * MAX_ENTITY_SIZE + x;
            if (currentModel.heights[modelIndex] > 0) {
                int mapIndex = (cy + y) * PREVIEW_MAP_SIZE + (cx + x);
                // Ensure bounds
                if (mapIndex >= 0 && mapIndex < PREVIEW_MAP_SIZE*PREVIEW_MAP_SIZE) {
                    t->heightmapRaw[mapIndex] = LEVEL_WATER + currentModel.heights[modelIndex];
                    t->colormapData[mapIndex] = currentModel.colors[modelIndex];
                }
            }
        }
    }
}

void RunEditor(void) {
    InitWindow(1600, 900, "Vortex Entity Editor");
    SetTargetFPS(60);

    InitEditorModel();

    // Override game settings for preview map size
    int oldMapSize = gameSettings.mapSize;
    gameSettings.mapSize = PREVIEW_MAP_SIZE;

    // Setup Preview Engine State
    EngineState state = {0};
    state.camera_x = PREVIEW_MAP_SIZE / 2.0f;
    state.camera_y = PREVIEW_MAP_SIZE / 2.0f + 40.0f;
    state.camera_z = 100.0f;
    state.horizon = 100.0f;
    state.phi = 0;

    // Setup Preview Terrain
    Terrain terrain;
    terrain.heightmapRaw = (unsigned char*)malloc(PREVIEW_MAP_SIZE * PREVIEW_MAP_SIZE);
    terrain.colormapData = (Color*)malloc(PREVIEW_MAP_SIZE * PREVIEW_MAP_SIZE * sizeof(Color));

    Renderer renderer;
    InitRenderer(&renderer);

    // Main Loop
    while (!WindowShouldClose()) {
        // Input
        HandleGridInput();

        // Camera Controls (Orbit)
        static float orbitAngle = 1.5707f; // Start South (PI/2)
        static float radius = 45.0f;

        if (IsKeyDown(KEY_UP)) state.camera_z += 2.0f;
        if (IsKeyDown(KEY_DOWN)) state.camera_z -= 2.0f;
        if (IsKeyDown(KEY_LEFT)) orbitAngle -= 0.02f;
        if (IsKeyDown(KEY_RIGHT)) orbitAngle += 0.02f;

        float cx = PREVIEW_MAP_SIZE / 2.0f;
        float cy = PREVIEW_MAP_SIZE / 2.0f;
        state.camera_x = cx + cosf(orbitAngle) * radius;
        state.camera_y = cy + sinf(orbitAngle) * radius;

        // Look at center (phi=0 is North/Negative Y)
        state.phi = orbitAngle - 1.5707f;

        state.sinphi = sinf(state.phi);
        state.cosphi = cosf(state.phi);

        // Update Preview
        UpdatePreviewTerrain(&terrain);

        // Render 3D View
        ClearFrameBuffer(&renderer, renderer.sky_color);
        DrawVertexSpace(&renderer, &state, &terrain);
        UpdateRendererTexture(&renderer);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Draw 3D Preview on Right
        Rectangle viewRect = { 800, 0, 800, 900 };
        DrawTexturePro(renderer.screenTexture,
                       (Rectangle){0, 0, GAME_WIDTH, GAME_HEIGHT},
                       viewRect, (Vector2){0,0}, 0.0f, WHITE);
        DrawRectangleLinesEx(viewRect, 2, BLACK);

        // Draw Editor UI on Left
        DrawEditorGrid();

        // Controls UI
        int uiX = 20;
        int uiY = 800;
        DrawText("LMB: Paint  RMB: Erase", uiX, uiY, 20, BLACK);
        DrawText("Arrows: Preview", uiX, uiY + 30, 20, BLACK);

        // Height Control
        DrawText(TextFormat("Height: %d", selectedHeight), uiX + 250, uiY, 20, BLACK);
        float hVal = (float)selectedHeight;
        hVal = GuiSlider((Rectangle){uiX + 380, uiY, 150, 20}, "H", hVal, 1, MAX_ENTITY_SIZE);
        selectedHeight = (int)hVal;

        // Palette
        int palX = uiX + 580;
        int palY = uiY - 10;
        int swatchSize = 24;

        for(int i=0; i<32; i++) {
            int px = i % 8;
            int py = i / 8;

            Rectangle r = { palX + px * swatchSize, palY + py * swatchSize, swatchSize, swatchSize };

            DrawRectangleRec(r, PALETTE[i]);
            DrawRectangleLinesEx(r, 1, BLACK);

            // Selection highlight
            if (selectedColor.r == PALETTE[i].r && selectedColor.g == PALETTE[i].g && selectedColor.b == PALETTE[i].b) {
                DrawRectangleLinesEx(r, 2, WHITE);
            }

            // Interaction
            if (CheckCollisionPointRec(GetMousePosition(), r)) {
                DrawRectangleLinesEx(r, 2, RED);
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    selectedColor = PALETTE[i];
                }
            }
        }

        // Selected Color Preview
        DrawRectangle(palX - 50, palY, 40, 40, selectedColor);
        DrawRectangleLines(palX - 50, palY, 40, 40, BLACK);

        // Size Controls
        DrawText(TextFormat("Size: %dx%d", currentModel.width, currentModel.length), uiX + 200, 20, 20, BLACK);
        if (GuiButton((Rectangle){uiX + 320, 20, 20, 20}, "+")) { currentModel.width++; currentModel.length++; }
        if (GuiButton((Rectangle){uiX + 350, 20, 20, 20}, "-")) { currentModel.width--; currentModel.length--; }
        if (currentModel.width > MAX_ENTITY_SIZE) currentModel.width = MAX_ENTITY_SIZE;
        if (currentModel.width < 1) currentModel.width = 1;
        currentModel.length = currentModel.width;

        EndDrawing();
    }

    // Cleanup
    gameSettings.mapSize = oldMapSize;
    free(terrain.heightmapRaw);
    free(terrain.colormapData);
    CloseRenderer(&renderer);
    CloseWindow();
}

bool GuiButton(Rectangle bounds, const char* text) {
    Vector2 mousePoint = GetMousePosition();
    bool clicked = false;

    Color color = LIGHTGRAY;
    if (CheckCollisionPointRec(mousePoint, bounds)) {
        color = GRAY;
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) clicked = true;
    }

    DrawRectangleRec(bounds, color);
    DrawRectangleLinesEx(bounds, 1, DARKGRAY);

    int textWidth = MeasureText(text, 10);
    DrawText(text, bounds.x + bounds.width/2 - textWidth/2, bounds.y + bounds.height/2 - 5, 10, BLACK);

    return clicked;
}

float GuiSlider(Rectangle bounds, const char* text, float value, float min, float max) {
    Vector2 mouse = GetMousePosition();

    // Draw Background
    DrawRectangleRec(bounds, LIGHTGRAY);
    DrawRectangleLinesEx(bounds, 1, DARKGRAY);

    // Handle Input
    if (CheckCollisionPointRec(mouse, bounds)) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            float mouseRatio = (mouse.x - bounds.x) / bounds.width;
            value = min + mouseRatio * (max - min);
        }
    }
    // Clamp
    if (value < min) value = min;
    if (value > max) value = max;

    // Draw Handle
    float ratio = (value - min) / (max - min);
    DrawRectangle(bounds.x + ratio * bounds.width - 5, bounds.y - 2, 10, bounds.height + 4, DARKGRAY);

    // Text
    DrawText(text, bounds.x - 20, bounds.y + 5, 10, BLACK);
    DrawText(TextFormat("%d", (int)value), bounds.x + bounds.width + 10, bounds.y + 5, 10, BLACK);

    return value;
}
