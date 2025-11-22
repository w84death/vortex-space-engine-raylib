#include "raylib.h"
#include "engine.h"
#include "terrain.h"
#include "renderer.h"
#include "input.h"
#include "ui.h"
#include "entities.h"
#include <stdlib.h>

void SpawnEntitySmart(EntityManager *manager, const Terrain *terrain, EntityType type, int count) {
    int spawned = 0;
    int attempts = 0;
    while(spawned < count && attempts < count * 1000) {
        attempts++;
        int x = GetRandomValue(0, MAP_SIZE - 1);
        int y = GetRandomValue(0, MAP_SIZE - 1);
        int index = y * MAP_SIZE + x;
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

int main(void)
{
    EngineState engineState;
    Terrain terrain;
    Renderer renderer;
    EntityManager *entityManager = (EntityManager*)malloc(sizeof(EntityManager));

    InitEngine(&engineState);
    InitRenderer(&renderer);

    DrawLoadingMessage("Generating Terrain (Please Wait)...");
    GenerateProceduralTerrain(&terrain);

    InitEntityManager(entityManager);

    SpawnEntitySmart(entityManager, &terrain, ENTITY_SHIP, 50);
    SpawnEntitySmart(entityManager, &terrain, ENTITY_UNIT, 200);
    SpawnEntitySmart(entityManager, &terrain, ENTITY_BUILDING, 100);


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
