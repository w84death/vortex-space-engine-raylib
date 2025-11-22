// VERTEX SPACE TERRAIN RENDERER
// Powered by Raylib

#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080
#define GAME_WIDTH 1280
#define GAME_HEIGHT 720  // 16:9 aspect ratio

#define MAX_PLANES 1024
#define MAP_Z_SCALE 256.0f
#define MOVE_SPEED 140.0f
#define LOD_FACTOR 512

// Mouselook Settings
#define MOUSE_SENSITIVITY_X 0.003f
#define MOUSE_SENSITIVITY_Y 2.0f

// procedural constants
// MUST BE POWER OF TWO (1024, 2048, 4096, 8192)
#define MAP_SIZE 8192
#define NOISE_SCALE 14.0f

// Global variables
Color *frameBuffer;
unsigned char *heightmapRaw;
Image heightmap;
Image colormap;
Color *heightmapData;
Color *colormapData;
int y_buffer[GAME_WIDTH];

// Fog parameters
float fog_start = (float)MAX_PLANES/3;
float fog_end = (float)MAX_PLANES;
float fog_density = 3.5f;
Color sky_color = {135, 206, 235, 255};

// Camera parameters
float camera_x = MAP_SIZE / 2.0f;
float camera_y = MAP_SIZE / 2.0f;
float camera_z = 256.0f;
float horizon = 100.0f;
float phi = 0.0f;
float sinphi, cosphi;

// Input State
bool cursorLocked = true;

// Terrain Colors
Color waterDeep = {0, 50, 120, 255};
Color waterShallow = {0, 100, 180, 255};
Color sand = {210, 180, 140, 255};
Color grassLow = {34, 139, 34, 255};
Color grassHigh = {0, 90, 0, 255};
Color rock = {90, 90, 90, 255};
Color snow = {255, 255, 255, 255};

// Function prototypes
void DrawVertexSpace(void);
Color ApplyFog(Color color, float fog_factor);
void GenerateProceduralTerrain(void);
void DrawMessage(const char* text);
void GenerateTerrainPixel(Color *colPixel, unsigned char *hPixel);

// The Code
int main(void)
{
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Vertex Space - Huge Terrain");

  frameBuffer = (Color*)malloc(GAME_WIDTH * GAME_HEIGHT * sizeof(Color));

  Image blank = GenImageColor(GAME_WIDTH, GAME_HEIGHT, BLACK);
  Texture2D screenTexture = LoadTextureFromImage(blank);
  UnloadImage(blank);

  ToggleFullscreen();

  SetTextureFilter(screenTexture, TEXTURE_FILTER_POINT);

  GenerateProceduralTerrain();

  DisableCursor();
  SetTargetFPS(60);

  // Main game loop
  while (!WindowShouldClose())
  {
    float deltaTime = GetFrameTime();
    sinphi = sinf(phi);
    cosphi = cosf(phi);

    if (IsKeyPressed(KEY_R)) {
      UnloadImage(heightmap);
      UnloadImage(colormap);
      GenerateProceduralTerrain();
      camera_x = MAP_SIZE / 2.0f;
      camera_y = MAP_SIZE / 2.0f;
    }

    if (IsKeyPressed(KEY_TAB)) {
      cursorLocked = !cursorLocked;
      if (cursorLocked) DisableCursor();
      else EnableCursor();
    }

    if (cursorLocked) {
      Vector2 mouseDelta = GetMouseDelta();
      phi -= mouseDelta.x * MOUSE_SENSITIVITY_X;
      horizon -= mouseDelta.y * MOUSE_SENSITIVITY_Y;
      if (horizon < -150.0f) horizon = -150.0f;
      if (horizon > GAME_HEIGHT + 150.0f) horizon = GAME_HEIGHT + 150.0f;
    }

    if (IsKeyDown(KEY_W)) {
      camera_x -= sinphi * MOVE_SPEED * deltaTime;
      camera_y -= cosphi * MOVE_SPEED * deltaTime;
    }
    if (IsKeyDown(KEY_S)) {
      camera_x += sinphi * MOVE_SPEED * deltaTime;
      camera_y += cosphi * MOVE_SPEED * deltaTime;
    }
    if (IsKeyDown(KEY_A)) {
      camera_x -= cosphi * MOVE_SPEED * deltaTime;
      camera_y += sinphi * MOVE_SPEED * deltaTime;
    }
    if (IsKeyDown(KEY_D)) {
      camera_x += cosphi * MOVE_SPEED * deltaTime;
      camera_y -= sinphi * MOVE_SPEED * deltaTime;
    }
    if (IsKeyDown(KEY_Q) || IsKeyDown(KEY_LEFT_SHIFT)) {
      camera_z -= MOVE_SPEED * deltaTime;
      if (camera_z < 10.0f) camera_z = 10.0f;
    }
    if (IsKeyDown(KEY_E) || IsKeyDown(KEY_SPACE)) {
      camera_z += MOVE_SPEED * deltaTime;
    }

    if (camera_x < 0) camera_x = 0;
    if (camera_x >= MAP_SIZE) camera_x = MAP_SIZE - 1;
    if (camera_y < 0) camera_y = 0;
    if (camera_y >= MAP_SIZE) camera_y = MAP_SIZE - 1;

    while (phi > 2.0f * PI) phi -= 2.0f * PI;
    while (phi < 0.0f) phi += 2.0f * PI;

    for (int i = 0; i < GAME_WIDTH * GAME_HEIGHT; i++) {
      frameBuffer[i] = sky_color;
    }
    DrawVertexSpace();
    UpdateTexture(screenTexture, frameBuffer);

    BeginDrawing();
      ClearBackground(BLACK);
      Rectangle srcRect = { 0.0f, 0.0f, (float)GAME_WIDTH, (float)GAME_HEIGHT };
      Rectangle destRect = { 0.0f, 0.0f, (float)GetScreenWidth(), (float)GetScreenHeight() };
      DrawTexturePro(screenTexture, srcRect, destRect, (Vector2){ 0, 0 }, 0.0f, WHITE);
      DrawText(TextFormat("FPS: %d", GetFPS()), 10, 10, 20, BLACK);
      DrawText(TextFormat("Pos: %.0f, %.0f", camera_x, camera_y), 10, 35, 20, BLACK);
      if (!cursorLocked) DrawText("MOUSE UNLOCKED (PRESS TAB)", 10, 60, 20, RED);

    EndDrawing();
  }

  UnloadImage(heightmap);
  UnloadImage(colormap);
  CloseWindow();

  return 0;
}

void DrawMessage(const char* text) {
  BeginDrawing();
  ClearBackground(RAYWHITE);
  DrawText(text, 20, 20, 20, DARKGRAY);
  EndDrawing();
}

void GenerateProceduralTerrain(void)
{
    DrawMessage("Generating Terrain (Please Wait)...");

    int offsetX = GetRandomValue(0, 10000);
    int offsetY = GetRandomValue(0, 10000);

    heightmap = GenImagePerlinNoise(MAP_SIZE, MAP_SIZE, offsetX, offsetY, NOISE_SCALE);

    ImageFormat(&heightmap, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    heightmapData = (Color*)heightmap.data;

    heightmapRaw = (unsigned char *)malloc(MAP_SIZE * MAP_SIZE);
    for (int i = 0; i < MAP_SIZE * MAP_SIZE; i++) {
        heightmapRaw[i] = heightmapData[i].r;
    }

    for (int i = 0; i < MAP_SIZE * MAP_SIZE; i++) {
        float h_normalized = heightmapData[i].r / 255.0f;
        float h_curved = powf(h_normalized, 3.0f);
        heightmapRaw[i] = (unsigned char)(h_curved * 255.0f);
    }

    colormap = GenImageColor(MAP_SIZE, MAP_SIZE, BLACK);
    ImageFormat(&colormap, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    colormapData = (Color*)colormap.data;

    for (int y = 0; y < MAP_SIZE; y++)
    {
      for (int x = 0; x < MAP_SIZE; x++)
      {
        int i = y * MAP_SIZE + x;
        GenerateTerrainPixel(&colormapData[i], &heightmapRaw[i]);
      }
    }
}

void GenerateTerrainPixel(Color *colPixel, unsigned char *hPixel)
{
  unsigned char h = *hPixel;

  if (h < 60) {
    *hPixel = 60;
    if (h < 40) *colPixel = waterDeep;
    else *colPixel = waterShallow;
    return;
  }
  else if (h < 95 + GetRandomValue(-5, 15)) {
    *colPixel = sand;
    colPixel->r += GetRandomValue(-10, 10);
    colPixel->g += GetRandomValue(-10, 10);
  }
  else if (h < 120 + GetRandomValue(-10, 25)) {
    *colPixel = grassLow;
    colPixel->g += GetRandomValue(-15, 15);
  }
  else if (h < 140 + GetRandomValue(-20, 50)) {
    *colPixel = grassHigh;
    colPixel->g += GetRandomValue(-25, 10);
    *hPixel += GetRandomValue(0, 10);
  }
  else if (h < 215 + GetRandomValue(-10, 10)) {
    *colPixel = rock;
    int r = GetRandomValue(-10, 10);
    colPixel->r += r;
    colPixel->g += r;
    colPixel->b += r;
  }else {
    *colPixel = snow;
  }
}

void DrawVertexSpace(void)
{
  for (int i = 0; i < GAME_WIDTH; i++) {
    y_buffer[i] = GAME_HEIGHT;
  }

  float pleft_x, pleft_y, pright_x, pright_y;

  for (int p = 1; p < MAX_PLANES; p++)
  {
    int step = 1 + (p / LOD_FACTOR);
    float fog_factor = 0.0f;
    if (p > fog_start) {
        float normalized_distance = ((float)p - fog_start) / (fog_end - fog_start);
        if (normalized_distance > 1.0f) normalized_distance = 1.0f;
        fog_factor = 1.0f - expf(-normalized_distance * fog_density);
    }

    pleft_x = (-cosphi * p - sinphi * p) + camera_x;
    pleft_y = (sinphi * p - cosphi * p) + camera_y;
    pright_x = (cosphi * p - sinphi * p) + camera_x;
    pright_y = (-sinphi * p - cosphi * p) + camera_y;

    float base_dx = (pright_x - pleft_x) / GAME_WIDTH;
    float base_dy = (pright_y - pleft_y) / GAME_WIDTH;

    float map_dx = base_dx * step;
    float map_dy = base_dy * step;

    float cur_map_x = pleft_x;
    float cur_map_y = pleft_y;

    for (int screen_x = 0; screen_x < GAME_WIDTH; screen_x += step)
    {
      int fill_width = (screen_x + step > GAME_WIDTH) ? (GAME_WIDTH - screen_x) : step;

      int lowest_horizon = y_buffer[screen_x];
      if (step > 1) {
        for (int k = 1; k < fill_width; k++) {
          if (y_buffer[screen_x + k] > lowest_horizon) {
            lowest_horizon = y_buffer[screen_x + k];
          }
        }
      }

      if (lowest_horizon <= 0) {
        cur_map_x += map_dx;
        cur_map_y += map_dy;
        continue;
      }

      int map_x_int = (int)cur_map_x & (MAP_SIZE - 1);
      int map_y_int = (int)cur_map_y & (MAP_SIZE - 1);
      int index = map_y_int * MAP_SIZE + map_x_int;

      int height = heightmapRaw[index];
      int screen_y = (int)((camera_z - height) / (float)p * MAP_Z_SCALE + horizon);

      if (screen_y < lowest_horizon){
        if (screen_y < 0) screen_y = 0;
        int draw_height = lowest_horizon - screen_y;

        if (draw_height > 0){
          Color col = colormapData[index];
          col = ApplyFog(col, fog_factor);

          for (int k = 0; k < fill_width; k++) {
            for (int y = screen_y; y < lowest_horizon; y++) {
              frameBuffer[y * GAME_WIDTH + (screen_x + k)] = col;
            }
          }

          for (int k = 0; k < fill_width; k++) {
            y_buffer[screen_x + k] = screen_y;
          }
        }
      }

      cur_map_x += map_dx;
      cur_map_y += map_dy;
    }
  }
}

Color ApplyFog(Color terrainColor, float fogFactor)
{
  if (fogFactor <= 0.0f) return terrainColor;
  if (fogFactor >= 1.0f) return sky_color;

  int alpha = (int)(fogFactor * 256);
  int inv_alpha = 256 - alpha;

  Color result;
  result.r = (unsigned char)((terrainColor.r * inv_alpha + sky_color.r * alpha) >> 8);
  result.g = (unsigned char)((terrainColor.g * inv_alpha + sky_color.g * alpha) >> 8);
  result.b = (unsigned char)((terrainColor.b * inv_alpha + sky_color.b * alpha) >> 8);
  result.a = 255;

  return result;
}
