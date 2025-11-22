#include "terrain.h"
#include <stdlib.h>
#include <math.h>

// Terrain Colors
static const Color waterDeep = {0, 50, 120, 255};
static const Color waterShallow = {0, 100, 180, 255};
static const Color sand = {210, 180, 140, 255};
static const Color grassLow = {34, 139, 34, 255};
static const Color grassHigh = {0, 90, 0, 255};
static const Color rock = {90, 90, 90, 255};
static const Color snow = {255, 255, 255, 255};

static void GenerateTerrainPixel(Color *colPixel, unsigned char *hPixel)
{
  unsigned char h = *hPixel;

  if (h < LEVEL_WATER) {
    *hPixel = LEVEL_WATER;
    if (h < LEVEL_WATER - 20) *colPixel = waterDeep;
    else *colPixel = waterShallow;
    return;
  }
  else if (h < LEVEL_SAND + GetRandomValue(-5, 15)) {
    *colPixel = sand;
    colPixel->r += GetRandomValue(-10, 10);
    colPixel->g += GetRandomValue(-10, 10);
  }
  else if (h < LEVEL_GRASS_LOW + GetRandomValue(-10, 25)) {
    *colPixel = grassLow;
    colPixel->g += GetRandomValue(-15, 15);
  }
  else if (h < LEVEL_GRASS_HIGH + GetRandomValue(-20, 50)) {
    *colPixel = grassHigh;
    colPixel->g += GetRandomValue(-25, 10);
    *hPixel += GetRandomValue(0, 10);
  }
  else if (h < LEVEL_ROCK + GetRandomValue(-10, 10)) {
    *colPixel = rock;
    int r = GetRandomValue(-10, 10);
    colPixel->r += r;
    colPixel->g += r;
    colPixel->b += r;
  }else {
    *colPixel = snow;
  }
}

void GenerateProceduralTerrain(Terrain *terrain)
{
    // Note: DrawMessage moved to calling code to decouple UI from Logic

    int offsetX = GetRandomValue(0, 10000);
    int offsetY = GetRandomValue(0, 10000);

    Image heightmap = GenImagePerlinNoise(MAP_SIZE, MAP_SIZE, offsetX, offsetY, NOISE_SCALE);

    ImageFormat(&heightmap, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    Color *heightmapData = (Color*)heightmap.data;

    terrain->heightmapRaw = (unsigned char *)malloc(MAP_SIZE * MAP_SIZE);
    for (int i = 0; i < MAP_SIZE * MAP_SIZE; i++) {
        terrain->heightmapRaw[i] = heightmapData[i].r;
    }

    UnloadImage(heightmap);
    heightmapData = NULL;

    for (int i = 0; i < MAP_SIZE * MAP_SIZE; i++) {
        float h_normalized = terrain->heightmapRaw[i] / 255.0f;
        float h_curved = powf(h_normalized, 3.0f);
        terrain->heightmapRaw[i] = (unsigned char)(h_curved * 255.0f);
    }

    terrain->colormap = GenImageColor(MAP_SIZE, MAP_SIZE, BLACK);
    ImageFormat(&terrain->colormap, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    terrain->colormapData = (Color*)terrain->colormap.data;

    for (int y = 0; y < MAP_SIZE; y++)
    {
      for (int x = 0; x < MAP_SIZE; x++)
      {
        int i = y * MAP_SIZE + x;
        GenerateTerrainPixel(&terrain->colormapData[i], &terrain->heightmapRaw[i]);
      }
    }
}

void UnloadTerrain(Terrain *terrain) {
    if (terrain->heightmapRaw) {
        free(terrain->heightmapRaw);
        terrain->heightmapRaw = NULL;
    }
    UnloadImage(terrain->colormap);
    terrain->colormapData = NULL;
}