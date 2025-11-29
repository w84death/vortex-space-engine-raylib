#include "terrain.h"
#include "settings.h"
#include <stdlib.h>
#include <math.h>

static void GenerateTerrainPixel(Color *colPixel, unsigned char *hPixel)
{
  unsigned char h = *hPixel;

  if (h < LEVEL_WATER) {
    *hPixel = LEVEL_WATER;
    if (h < LEVEL_WATER - 12) *colPixel = DB_VENICE_BLUE;
    else *colPixel = DB_ROYAL_BLUE;
    return;
  }
  else if (h < LEVEL_SAND + GetRandomValue(-5, 15)) {
    if (h < LEVEL_WATER + 4) {
        *colPixel = DB_TWINE;
    } else {
        *colPixel = DB_PANCHO;
    }
  }
  else if (h < LEVEL_GRASS_LOW + GetRandomValue(-10, 25)) {
    *colPixel = (Color[]){DB_CHRISTI, DB_ELF_GREEN}[GetRandomValue(0, 1)];
  }
  else if (h < LEVEL_GRASS_HIGH + GetRandomValue(-20, 50)) {
    *colPixel = (Color[]){ DB_APPLE_BLOSSOM, DB_DELL, DB_VERDUN_GREEN, DB_SAPLING }[GetRandomValue(0, 3)];
    *hPixel += GetRandomValue(0, 10);
  }
  else if (h < LEVEL_ROCK + GetRandomValue(-10, 10)) {
    *colPixel = DB_SHUTTLE_GREY;
  }else {
    *colPixel = DB_WHITE;
  }
}

void GenerateProceduralTerrain(Terrain *terrain)
{
    // Note: DrawMessage moved to calling code to decouple UI from Logic

    int offsetX = GetRandomValue(0, 10000);
    int offsetY = GetRandomValue(0, 10000);

    Image heightmap = GenImagePerlinNoise(gameSettings.mapSize, gameSettings.mapSize, offsetX, offsetY, gameSettings.noiseScale);

    ImageFormat(&heightmap, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    Color *heightmapData = (Color*)heightmap.data;

    terrain->heightmapRaw = (unsigned char *)malloc(gameSettings.mapSize * gameSettings.mapSize);
    for (int i = 0; i < gameSettings.mapSize * gameSettings.mapSize; i++) {
        terrain->heightmapRaw[i] = heightmapData[i].r;
    }

    UnloadImage(heightmap);
    heightmapData = NULL;

    for (int i = 0; i < gameSettings.mapSize * gameSettings.mapSize; i++) {
        float h_normalized = terrain->heightmapRaw[i] / 255.0f;
        float h_curved = powf(h_normalized, 3.0f);
        terrain->heightmapRaw[i] = (unsigned char)(h_curved * 255.0f);
    }

    terrain->colormap = GenImageColor(gameSettings.mapSize, gameSettings.mapSize, BLACK);
    ImageFormat(&terrain->colormap, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    terrain->colormapData = (Color*)terrain->colormap.data;

    for (int y = 0; y < gameSettings.mapSize; y++)
    {
      for (int x = 0; x < gameSettings.mapSize; x++)
      {
        int i = y * gameSettings.mapSize + x;
        GenerateTerrainPixel(&terrain->colormapData[i], &terrain->heightmapRaw[i]);
      }
    }

    // Fast Gradient-Based Lighting
    // Simulates light coming from Top-Left (North-West)
    // We calculate the slope between current pixel and Right/Bottom neighbors.
    float shadowStrength = 2.5f;

    for (int y = 0; y < gameSettings.mapSize - 1; y++)
    {
      for (int x = 0; x < gameSettings.mapSize - 1; x++)
      {
         int i = y * gameSettings.mapSize + x;

         // Current height
         int h = terrain->heightmapRaw[i];

         if (((h < LEVEL_GRASS_LOW ) && (h > LEVEL_WATER)) || (h >= LEVEL_ROCK)){

          // Neighbors (Right and Down)
          int hRight = terrain->heightmapRaw[i + 1];
          int hDown = terrain->heightmapRaw[i + gameSettings.mapSize];

          // Calculate slope (gradient)
          // If hRight > h, terrain slopes UP to the RIGHT (faces Left/West).
          // If hDown > h, terrain slopes UP to the BOTTOM (faces Up/North).
          // Since light is from Top-Left, these surfaces catch light.
          int diffX = hRight - h;
          int diffY = hDown - h;

          int lightVal = (int)((diffX + diffY) * shadowStrength);

          Color *col = &terrain->colormapData[i];

          if (lightVal < 0) {
              // Shadow: darker and more saturated
              float factor = 1.0f + (lightVal * 0.04f);
              if (factor < 0.25f) factor = 0.25f;

              float r = (float)col->r * factor;
              float g = (float)col->g * factor;
              float b = (float)col->b * factor;

              // Saturation boost
              float gray = (r + g + b) / 3.0f;
              float satAmount = 1.4f;

              r = gray + (r - gray) * satAmount;
              g = gray + (g - gray) * satAmount;
              b = gray + (b - gray) * satAmount;

              if (r < 0) r = 0;
              if (r > 255) r = 255;
              if (g < 0) g = 0;
              if (g > 255) g = 255;
              if (b < 0) b = 0;
              if (b > 255) b = 255;

              col->r = (unsigned char)r;
              col->g = (unsigned char)g;
              col->b = (unsigned char)b;
          } else {
              // Light: additive brightness
              int r = col->r + lightVal;
              int g = col->g + lightVal;
              int b = col->b + lightVal;

              if (r > 255) r = 255;
              if (g > 255) g = 255;
              if (b > 255) b = 255;

              col->r = (unsigned char)r;
              col->g = (unsigned char)g;
              col->b = (unsigned char)b;
          }
        }
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
