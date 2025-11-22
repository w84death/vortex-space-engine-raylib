#include "raylib.h"
#include "engine.h"
#include "terrain.h"
#include "renderer.h"
#include "input.h"
#include "ui.h"
#include "entities.h"
#include <stdlib.h>

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
    for (int i = 0; i < 1024; i++) {
        AddEntity(entityManager, ENTITY_SHIP, (float)GetRandomValue(0, MAP_SIZE), (float)GetRandomValue(0, MAP_SIZE));
    }
    for (int i = 0; i < 1024; i++) {
        AddEntity(entityManager, ENTITY_UNIT, (float)GetRandomValue(0, MAP_SIZE), (float)GetRandomValue(0, MAP_SIZE));
    }
    for (int i = 0; i < 512; i++) {
        AddEntity(entityManager, ENTITY_BUILDING, (float)GetRandomValue(0, MAP_SIZE), (float)GetRandomValue(0, MAP_SIZE));
    }

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
