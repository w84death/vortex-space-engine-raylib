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
#define MOVE_SPEED 180.0f
#define LOD_FACTOR 512

// Mouselook Settings
#define MOUSE_SENSITIVITY_X 0.003f
#define MOUSE_SENSITIVITY_Y 2.0f

// procedural constants
// MUST BE POWER OF TWO (1024, 2048, 4096, 8192)
#define MAP_SIZE 8192
#define NOISE_SCALE 18.0f

// Global variables
Color *frameBuffer;
unsigned char *heightmapRaw;
Image heightmap;
Image colormap;
Color *heightmapData;
Color *colormapData;
int y_buffer[GAME_WIDTH];
float depth_scale_table[MAX_PLANES];

// Fixed-point math constants
#define FIXED_POINT_SHIFT 16
#define FIXED_POINT_SCALE (1 << FIXED_POINT_SHIFT)

// Fog parameters
Color sky_color = {135, 206, 235, 255};

// Camera parameters
float camera_x = MAP_SIZE / 2.0f;
float camera_y = MAP_SIZE / 2.0f;
float camera_z = 600.0f;
float horizon = -150.0f;
float phi = 0.785398f;
float sinphi, cosphi;

// Input State
bool cursorLocked = false;

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

  SetTargetFPS(60);

  // Main game loop
  while (!WindowShouldClose())
  {
    float deltaTime = GetFrameTime();
    sinphi = sinf(phi);
    cosphi = cosf(phi);

    if (IsKeyPressed(KEY_R)) {
      free(heightmapRaw);
      UnloadImage(colormap);
      GenerateProceduralTerrain();
      camera_x = MAP_SIZE / 2.0f;
      camera_y = MAP_SIZE / 2.0f;
    }

    // Edge Scrolling
    Vector2 mousePos = GetMousePosition();
    int edgeSize = 40;

    if (mousePos.x < edgeSize) { // Left
      camera_x -= cosphi * MOVE_SPEED * deltaTime;
      camera_y += sinphi * MOVE_SPEED * deltaTime;
    }
    if (mousePos.x > GetScreenWidth() - edgeSize) { // Right
      camera_x += cosphi * MOVE_SPEED * deltaTime;
      camera_y -= sinphi * MOVE_SPEED * deltaTime;
    }
    if (mousePos.y < edgeSize) { // Top
      camera_x -= sinphi * MOVE_SPEED * deltaTime;
      camera_y -= cosphi * MOVE_SPEED * deltaTime;
    }
    if (mousePos.y > GetScreenHeight() - edgeSize) { // Bottom
      camera_x += sinphi * MOVE_SPEED * deltaTime;
      camera_y += cosphi * MOVE_SPEED * deltaTime;
    }

    // Mouse Zoom
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        camera_z -= wheel * 30.0f;
        if (camera_z < 50.0f) camera_z = 50.0f;
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
    if (IsKeyDown(KEY_Q)) {
      phi -= 1.5f * deltaTime;
    }
    if (IsKeyDown(KEY_E)) {
      phi += 1.5f * deltaTime;
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
      DrawText("Strategy View: WASD/Mouse edges to move, Q/E to rotate, Scroll to zoom", 10, 60, 20, DARKBLUE);

    EndDrawing();
  }

  free(heightmapRaw);
  free(frameBuffer);
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

    UnloadImage(heightmap);
    heightmapData = NULL;

    for (int i = 0; i < MAP_SIZE * MAP_SIZE; i++) {
        float h_normalized = heightmapRaw[i] / 255.0f;
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

  for(int i=1; i<MAX_PLANES; i++){
    depth_scale_table[i] = MAP_Z_SCALE / (float)i;
  }

  float pleft_x, pleft_y, pright_x, pright_y;

  for (int p = 1; p < MAX_PLANES; p++)
  {
    int step = 1 + (p / LOD_FACTOR);

    pleft_x = (-cosphi * p - sinphi * p) + camera_x;
    pleft_y = (sinphi * p - cosphi * p) + camera_y;
    pright_x = (cosphi * p - sinphi * p) + camera_x;
    pright_y = (-sinphi * p - cosphi * p) + camera_y;

    // Convert to fixed-point for faster coordinate calculations
    int pleft_x_fixed = (int)(pleft_x * FIXED_POINT_SCALE);
    int pleft_y_fixed = (int)(pleft_y * FIXED_POINT_SCALE);
    int pright_x_fixed = (int)(pright_x * FIXED_POINT_SCALE);
    int pright_y_fixed = (int)(pright_y * FIXED_POINT_SCALE);

    int base_dx_fixed = (pright_x_fixed - pleft_x_fixed) / GAME_WIDTH;
    int base_dy_fixed = (pright_y_fixed - pleft_y_fixed) / GAME_WIDTH;

    int map_dx_fixed = base_dx_fixed * step;
    int map_dy_fixed = base_dy_fixed * step;

    int cur_map_x_fixed = pleft_x_fixed;
    int cur_map_y_fixed = pleft_y_fixed;

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
        cur_map_x_fixed += map_dx_fixed;
        cur_map_y_fixed += map_dy_fixed;
        continue;
      }

      int map_x_int = (cur_map_x_fixed >> FIXED_POINT_SHIFT) & (MAP_SIZE - 1);
      int map_y_int = (cur_map_y_fixed >> FIXED_POINT_SHIFT) & (MAP_SIZE - 1);
      int index = map_y_int * MAP_SIZE + map_x_int;

      int height = heightmapRaw[index];
      int screen_y = (int)((camera_z - height) * depth_scale_table[p] + horizon);

      if (screen_y < lowest_horizon){
        if (screen_y < 0) screen_y = 0;
        int draw_height = lowest_horizon - screen_y;

        if (draw_height > 0){
          Color col = colormapData[index];

          int base_offset = screen_y * GAME_WIDTH + screen_x;
          for (int k = 0; k < fill_width; k++) {
            int offset = base_offset + k;
            for (int y = screen_y; y < lowest_horizon; y++) {
              frameBuffer[offset] = col;
              offset += GAME_WIDTH;
            }
          }

          for (int k = 0; k < fill_width; k++) {
            y_buffer[screen_x + k] = screen_y;
          }
        }
      }

      cur_map_x_fixed += map_dx_fixed;
      cur_map_y_fixed += map_dy_fixed;
    }
  }
}


