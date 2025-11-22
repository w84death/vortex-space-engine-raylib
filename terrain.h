#ifndef TERRAIN_H
#define TERRAIN_H

#include "raylib.h"
#include "constants.h"

typedef struct {
    unsigned char *heightmapRaw;
    Color *colormapData;
    Image colormap;
} Terrain;

void GenerateProceduralTerrain(Terrain *terrain);
void UnloadTerrain(Terrain *terrain);

#endif // TERRAIN_H