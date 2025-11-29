#include "raylib.h"
#include "engine.h"
#include "terrain.h"
#include "renderer.h"
#include "input.h"
#include "ui.h"
#include "entities.h"
#include "settings.h"
#include "editor.h"
#include <stdlib.h>

GameSettings gameSettings;

void SpawnEntitySmart(EntityManager *manager, const Terrain *terrain, EntityType type, int count) {
  int spawned = 0;
  int attempts = 0;
  while(spawned < count && attempts < count * 1000) {
    attempts++;
    int x = GetRandomValue(0, gameSettings.mapSize - 1);
    int y = GetRandomValue(0, gameSettings.mapSize - 1);
    int index = y * gameSettings.mapSize + x;
    unsigned char h = terrain->heightmapRaw[index];

    bool valid = false;
    if (type == ENTITY_SHIP) {
      if (h <= LEVEL_WATER) valid = true;
    } else if (type == ENTITY_UNIT || type == ENTITY_BUILDING) {
      if (h > LEVEL_WATER + 2 && h < LEVEL_SAND) valid = true;
    }

    if (valid) {
      AddEntity(manager, type, (float)x, (float)y);
      spawned++;
    }
  }
}

void RunSetup() {
  InitWindow(400, 400, "Game Setup");
  SetTargetFPS(60);

  int selection = 1; // Default 2048 (Small)
  bool confirmed = false;

  while (!WindowShouldClose()) {
      if (IsKeyPressed(KEY_UP)) selection--;
      if (IsKeyPressed(KEY_DOWN)) selection++;
      if (selection < 0) selection = 5;
      if (selection > 5) selection = 0;

      if (IsKeyPressed(KEY_ENTER)) {
          confirmed = true;
          break;
      }
      if (IsKeyPressed(KEY_ESCAPE)) {
          selection = 5;
          confirmed = true;
          break;
      }

      BeginDrawing();
      ClearBackground(THEME_BG);
      DrawRectangleLines(10, 10, 380, 380, THEME_ACCENT);

      DrawText("Select Mode / Map Size:", 20, 20, 20, THEME_TEXT);

      Color c0 = (selection == 0) ? THEME_ACCENT_LIGHT : THEME_TEXT_DIM;
      Color c1 = (selection == 1) ? THEME_ACCENT_LIGHT : THEME_TEXT_DIM;
      Color c2 = (selection == 2) ? THEME_ACCENT_LIGHT : THEME_TEXT_DIM;
      Color c3 = (selection == 3) ? THEME_ACCENT_LIGHT : THEME_TEXT_DIM;
      Color c4 = (selection == 4) ? THEME_ACCENT_LIGHT : THEME_TEXT_DIM;
      Color c5 = (selection == 5) ? THEME_ACCENT_LIGHT : THEME_TEXT_DIM;

      DrawText("1024x1024 (Tiny)", 40, 60, 20, c0);
      DrawText("2048x2048 (Small)", 40, 90, 20, c1);
      DrawText("4096x4096 (Medium)", 40, 120, 20, c2);
      DrawText("8192x8192 (Large)", 40, 150, 20, c3);

      DrawText("----------------", 40, 175, 20, THEME_TEXT);
      DrawText("Model Editor", 40, 200, 20, c4);
      DrawText("Quit System", 40, 230, 20, c5);

      DrawText("Press ENTER to Start", 20, 300, 20, THEME_ACCENT);
      DrawText("Use Arrow Keys and Enter", 20, 330, 16, THEME_TEXT_DIM);

      EndDrawing();
  }

  if (!confirmed) {
      gameSettings.gameMode = MODE_QUIT;
      CloseWindow();
      return;
  }

  // Set settings based on selection
  gameSettings.gameMode = MODE_GAME;

  switch(selection) {
    case 0: // 1024
      gameSettings.mapSize = 1024;
      gameSettings.noiseScale = 2.0f;
      gameSettings.shipCount = 10;
      gameSettings.unitCount = 40;
      gameSettings.buildingCount = 20;
      break;
    case 1: // 2048
      gameSettings.mapSize = 2048;
      gameSettings.noiseScale = 4.0f;
      gameSettings.shipCount = 20;
      gameSettings.unitCount = 120;
      gameSettings.buildingCount = 40;
      break;
    case 2: // 4096
      gameSettings.mapSize = 4096;
      gameSettings.noiseScale = 8.0f;
      gameSettings.shipCount = 100;
      gameSettings.unitCount = 250;
      gameSettings.buildingCount = 80;
      break;
    case 3: // 8192
      gameSettings.mapSize = 8192;
      gameSettings.noiseScale = 16.0f;
      gameSettings.shipCount = 500;
      gameSettings.unitCount = 1000;
      gameSettings.buildingCount = 100;
      break;
    case 4: // Editor
      gameSettings.gameMode = MODE_EDITOR;
      gameSettings.mapSize = 1024;
      break;
    case 5: // Quit
      gameSettings.gameMode = MODE_QUIT;
      break;
  }

  CloseWindow();
}

int main(void)
{
    RunSetup();

    if (gameSettings.gameMode == MODE_QUIT) return 0;

    if (gameSettings.gameMode == MODE_EDITOR) {
        RunEditor();
        return 0;
    }

    EngineState engineState;
    Terrain terrain;
    Renderer renderer;
    EntityManager *entityManager = (EntityManager*)malloc(sizeof(EntityManager));

    InitEngine(&engineState);
    InitRenderer(&renderer);

    DrawLoadingMessage("Generating Terrain (Please Wait)...");
    GenerateProceduralTerrain(&terrain);

    InitEntityManager(entityManager);

    SpawnEntitySmart(entityManager, &terrain, ENTITY_SHIP, gameSettings.shipCount);
    SpawnEntitySmart(entityManager, &terrain, ENTITY_UNIT, gameSettings.unitCount);
    SpawnEntitySmart(entityManager, &terrain, ENTITY_BUILDING, gameSettings.buildingCount);


    // Spawn Menu State
    bool showSpawnMenu = false;
    Vector2 spawnMenuPos = {0};
    int spawnMapX = 0;
    int spawnMapY = 0;
    int menuLevel = 0; // 0: Categories, 1: Models
    EntityType selectedCategory = ENTITY_UNIT;

    // Main game loop
    while (!WindowShouldClose())
    {
        UpdateEngine(&engineState);

        // Handle Spawn Menu Input
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            int mx = GetMouseX();
            int my = GetMouseY();
            int mapX, mapY;
            if (GetMapCoordinates(&renderer, &engineState, &terrain, mx, my, &mapX, &mapY)) {
                showSpawnMenu = true;
                spawnMenuPos = (Vector2){(float)mx, (float)my};
                spawnMapX = mapX;
                spawnMapY = mapY;
                menuLevel = 0;
            } else {
                showSpawnMenu = false;
            }
        }

        // Handle Menu Clicks
        if (showSpawnMenu && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mouse = GetMousePosition();
            bool clickedItem = false;

            if (menuLevel == 0) {
                 EntityType types[] = {ENTITY_UNIT, ENTITY_BUILDING, ENTITY_SHIP};
                 
                 for(int i=0; i<3; i++) {
                     Rectangle itemRect = {spawnMenuPos.x, spawnMenuPos.y + i*20, 150, 20};
                     if (CheckCollisionPointRec(mouse, itemRect)) {
                         menuLevel = 1;
                         selectedCategory = types[i];
                         clickedItem = true;
                     }
                 }
            } else {
                // Check items
                int displayIndex = 0;
                for(int i=0; i < modelRegistry.count; i++) {
                    VoxelModel *m = &modelRegistry.models[i];
                    if (m->type != selectedCategory) continue;

                    Rectangle itemRect = {spawnMenuPos.x, spawnMenuPos.y + displayIndex*20, 150, 20};
                    if (CheckCollisionPointRec(mouse, itemRect)) {
                        
                        // Validate
                        int index = spawnMapY * gameSettings.mapSize + spawnMapX;
                        unsigned char h = terrain.heightmapRaw[index];
                        bool valid = false;

                        if (m->type == ENTITY_SHIP && h <= LEVEL_WATER) valid = true;
                        if ((m->type == ENTITY_UNIT || m->type == ENTITY_BUILDING) && h > LEVEL_WATER) valid = true;

                        if (valid) {
                            AddEntityFromModel(entityManager, m->type, (float)spawnMapX, (float)spawnMapY, m);
                        }

                        clickedItem = true;
                        showSpawnMenu = false;
                    }
                    displayIndex++;
                }
            }
            if (!clickedItem) showSpawnMenu = false;
        }

        HandleInput(&engineState, &terrain);
        UpdateEntities(entityManager, engineState.deltaTime, &terrain);

        PaintEntities(entityManager, &terrain);
        ClearFrameBuffer(&renderer, renderer.sky_color);
        DrawVertexSpace(&renderer, &engineState, &terrain);
        RestoreEntities(entityManager, &terrain);

        UpdateRendererTexture(&renderer);

        BeginDrawing();
            DrawRendererTextureToScreen(&renderer);
            DrawGameUI(&engineState);

            if (showSpawnMenu) {
                // Draw Menu
                int menuW = 150;
                
                if (menuLevel == 0) {
                    // Draw Categories
                    int menuH = 3 * 20;
                    DrawRectangle(spawnMenuPos.x, spawnMenuPos.y, menuW, menuH, THEME_PANEL);
                    DrawRectangleLines(spawnMenuPos.x, spawnMenuPos.y, menuW, menuH, THEME_ACCENT);
                    
                    const char* categories[] = {"Units", "Buildings", "Ships"};
                    for(int i=0; i<3; i++) {
                         int y = spawnMenuPos.y + i*20;
                         Rectangle itemRect = {spawnMenuPos.x, y, menuW, 20};
                         bool hover = CheckCollisionPointRec(GetMousePosition(), itemRect);

                         if (hover) {
                            DrawRectangleRec(itemRect, THEME_GRID_LINE);
                         }
                         DrawText(categories[i], spawnMenuPos.x + 10, y + 5, 10, hover ? THEME_ACCENT_LIGHT : THEME_TEXT);
                    }
                } else {
                    // Draw Models
                    int count = 0;
                    for(int i=0; i<modelRegistry.count; i++) {
                        if (modelRegistry.models[i].type == selectedCategory) count++;
                    }

                    int menuH = (count > 0) ? count * 20 : 30;
                    DrawRectangle(spawnMenuPos.x, spawnMenuPos.y, menuW, menuH, THEME_PANEL);
                    DrawRectangleLines(spawnMenuPos.x, spawnMenuPos.y, menuW, menuH, THEME_ACCENT);

                    if (count == 0) {
                        DrawText("No Models", spawnMenuPos.x + 10, spawnMenuPos.y + 10, 10, THEME_TEXT_DIM);
                    }

                    int displayIndex = 0;
                    for(int i=0; i < modelRegistry.count; i++) {
                        VoxelModel *m = &modelRegistry.models[i];
                        if (m->type != selectedCategory) continue;

                        int y = spawnMenuPos.y + displayIndex*20;
                        Rectangle itemRect = {spawnMenuPos.x, y, menuW, 20};
                        
                        bool hover = CheckCollisionPointRec(GetMousePosition(), itemRect);
                        if (hover) {
                            DrawRectangleRec(itemRect, THEME_GRID_LINE);
                        }
                        
                        DrawText(m->name, spawnMenuPos.x + 10, y + 5, 10, hover ? THEME_ACCENT_LIGHT : THEME_TEXT);
                        displayIndex++;
                    }
                }
            }
        EndDrawing();
    }

    free(entityManager);
    UnloadTerrain(&terrain);
    CloseRenderer(&renderer);
    CloseEngine();

    return 0;
}
