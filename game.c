#include "raylib.h"
#include "engine.h"
#include "terrain.h"
#include "renderer.h"
#include "input.h"
#include "ui.h"

int main(void)
{
    EngineState engineState;
    Terrain terrain;
    Renderer renderer;

    InitEngine(&engineState);
    InitRenderer(&renderer);

    DrawLoadingMessage("Generating Terrain (Please Wait)...");
    GenerateProceduralTerrain(&terrain);

    // Main game loop
    while (!WindowShouldClose())
    {
        UpdateEngine(&engineState);
        HandleInput(&engineState, &terrain);

        ClearFrameBuffer(&renderer, renderer.sky_color);
        DrawVertexSpace(&renderer, &engineState, &terrain);
        UpdateRendererTexture(&renderer);

        BeginDrawing();
            DrawRendererTextureToScreen(&renderer);
            DrawGameUI(&engineState);
        EndDrawing();
    }

    UnloadTerrain(&terrain);
    CloseRenderer(&renderer);
    CloseEngine();

    return 0;
}