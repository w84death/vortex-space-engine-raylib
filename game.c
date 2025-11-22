#include "raylib.h"
#include "engine.h"
#include "terrain.h"
#include "renderer.h"
#include "input.h"
#include "ui.h"
#include "entities.h"
#include "settings.h"
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
            // Spawn on Sand layer
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

    int selection = 3; // Default 8192
    
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_UP)) selection--;
        if (IsKeyPressed(KEY_DOWN)) selection++;
        if (selection < 0) selection = 3;
        if (selection > 3) selection = 0;

        if (IsKeyPressed(KEY_ENTER)) break;

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Select Map Size:", 20, 20, 20, BLACK);

        Color c0 = (selection == 0) ? RED : DARKGRAY;
        Color c1 = (selection == 1) ? RED : DARKGRAY;
        Color c2 = (selection == 2) ? RED : DARKGRAY;
        Color c3 = (selection == 3) ? RED : DARKGRAY;

        DrawText("1024x1024 (Tiny)", 40, 60, 20, c0);
        DrawText("2048x2048 (Small)", 40, 90, 20, c1);
        DrawText("4096x4096 (Medium)", 40, 120, 20, c2);
        DrawText("8192x8192 (Large)", 40, 150, 20, c3);
        
        DrawText("Press ENTER to Start", 20, 300, 20, DARKBLUE);
        DrawText("Use Arrow Keys and Enter", 20, 330, 16, DARKGRAY);

        EndDrawing();
    }

    // Set settings based on selection
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
            gameSettings.noiseScale = 6.0f;
            gameSettings.shipCount = 20;
            gameSettings.unitCount = 80;
            gameSettings.buildingCount = 40;
            break;
        case 2: // 4096
            gameSettings.mapSize = 4096;
            gameSettings.noiseScale = 10.0f;
            gameSettings.shipCount = 40;
            gameSettings.unitCount = 150;
            gameSettings.buildingCount = 80;
            break;
        case 3: // 8192
            gameSettings.mapSize = 8192;
            gameSettings.noiseScale = 18.0f;
            gameSettings.shipCount = 50;
            gameSettings.unitCount = 200;
            gameSettings.buildingCount = 100;
            break;
    }

    CloseWindow();
}

int main(void)
{
    RunSetup();

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


    // Main game loop
    while (!WindowShouldClose())
    {
        UpdateEngine(&engineState);
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
        EndDrawing();
    }

    free(entityManager);
    UnloadTerrain(&terrain);
    CloseRenderer(&renderer);
    CloseEngine();

    return 0;
}
