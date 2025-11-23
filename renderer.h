#ifndef RENDERER_H
#define RENDERER_H

#include "raylib.h"
#include "constants.h"
#include "engine.h"
#include "terrain.h"

typedef struct {
    Color *frameBuffer;
    Texture2D screenTexture;
    int y_buffer[GAME_WIDTH];
    float depth_scale_table[MAX_PLANES];
    Color sky_color;
} Renderer;

void InitRenderer(Renderer *renderer);
void ClearFrameBuffer(Renderer *renderer, Color color);
void DrawVertexSpace(Renderer *renderer, const EngineState *state, const Terrain *terrain);
void UpdateRendererTexture(Renderer *renderer);
void DrawRendererTextureToScreen(Renderer *renderer);
bool GetMapCoordinates(const Renderer *renderer, const EngineState *state, const Terrain *terrain, int screenX, int screenY, int *outMapX, int *outMapY);
void CloseRenderer(Renderer *renderer);

#endif // RENDERER_H