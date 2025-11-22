// VERTEX SPACE TERRAIN RENDERER
// Powered by Raylib

#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080
#define GAME_WIDTH 640
#define GAME_HEIGHT 360  // 16:9 aspect ratio

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
#define NOISE_SCALE 25.0f

// Global variables
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
float horizon = 100.0f;        // Adjusted starting horizon
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
void GenerateTerrainPixel(Color *colPixel, Color *hPixel);

int main(void)
{
    // Initialization
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Vertex Space - Huge Terrain");
    ToggleFullscreen();

    RenderTexture2D target = LoadRenderTexture(GAME_WIDTH, GAME_HEIGHT);
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT); // Keep it pixelated!

    // Generate initial terrain
    GenerateProceduralTerrain();

    // --- MOUSE SETUP ---
    DisableCursor(); // Hides cursor and locks it to center
    SetTargetFPS(60);

    // Main game loop
    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();

        sinphi = sinf(phi);
        cosphi = cosf(phi);

        // --- INPUT HANDLING ---

        if (IsKeyPressed(KEY_R)) {
            UnloadImage(heightmap);
            UnloadImage(colormap);
            GenerateProceduralTerrain();
            camera_x = MAP_SIZE / 2.0f;
            camera_y = MAP_SIZE / 2.0f;
        }


        // Toggle Mouse Lock with TAB
        if (IsKeyPressed(KEY_TAB)) {
            cursorLocked = !cursorLocked;
            if (cursorLocked) DisableCursor();
            else EnableCursor();
        }

        // --- MOUSE LOOK ---
        if (cursorLocked) {
            Vector2 mouseDelta = GetMouseDelta();

            // Horizontal Rotation (Yaw)
            // Note: Subtraction makes mouse-right turn right in this coordinate system
            phi -= mouseDelta.x * MOUSE_SENSITIVITY_X;

            // Vertical Look (Pitch/Horizon)
            // In Voxel space, looking UP means moving the horizon DOWN (Higher Pixel Y)
            // Mouse Y is negative when moving up.
            // We add negative mouse Y to horizon?
            // Let's invert: Mouse UP (neg) -> Horizon moves DOWN (pos) -> Look UP
            horizon -= mouseDelta.y * MOUSE_SENSITIVITY_Y;

            // Clamp Horizon to prevent flipping over
            if (horizon < -150.0f) horizon = -150.0f;
            if (horizon > GAME_HEIGHT + 150.0f) horizon = GAME_HEIGHT + 150.0f;
        }

        // Keyboard Rotation (Optional backup)
        if (IsKeyDown(KEY_A)) phi += 1.5f * deltaTime;
        if (IsKeyDown(KEY_D)) phi -= 1.5f * deltaTime;

        // Movement
        if (IsKeyDown(KEY_W)) {
            camera_x -= sinphi * MOVE_SPEED * deltaTime;
            camera_y -= cosphi * MOVE_SPEED * deltaTime;
        }
        if (IsKeyDown(KEY_S)) {
            camera_x += sinphi * MOVE_SPEED * deltaTime;
            camera_y += cosphi * MOVE_SPEED * deltaTime;
        }

        // Altitude
        // Added SPACE/SHIFT for standard FPS feel, kept Q/E as well
        if (IsKeyDown(KEY_Q) || IsKeyDown(KEY_LEFT_SHIFT)) {
            camera_z -= MOVE_SPEED * deltaTime;
            if (camera_z < 10.0f) camera_z = 10.0f;
        }
        if (IsKeyDown(KEY_E) || IsKeyDown(KEY_SPACE)) {
            camera_z += MOVE_SPEED * deltaTime;
        }

        // --- BOUNDARY CLAMPING ---
        if (camera_x < 0) camera_x = 0;
        if (camera_x >= MAP_SIZE) camera_x = MAP_SIZE - 1;
        if (camera_y < 0) camera_y = 0;
        if (camera_y >= MAP_SIZE) camera_y = MAP_SIZE - 1;

        while (phi > 2.0f * PI) phi -= 2.0f * PI;
        while (phi < 0.0f) phi += 2.0f * PI;

        BeginTextureMode(target);
          ClearBackground(sky_color);
          DrawVertexSpace();
        EndTextureMode();

        BeginDrawing();
          ClearBackground(BLACK);
          Rectangle srcRect = { 0.0f, 0.0f, (float)target.texture.width, (float)-target.texture.height };
          Rectangle destRect = { 0.0f, 0.0f, (float)GetScreenWidth(), (float)GetScreenHeight() };
          DrawTexturePro(target.texture, srcRect, destRect, (Vector2){ 0, 0 }, 0.0f, WHITE);

          DrawText(TextFormat("FPS: %d", GetFPS()), 10, 10, 20, BLACK);
          DrawText(TextFormat("Pos: %.0f, %.0f", camera_x, camera_y), 10, 35, 20, BLACK);
          if (!cursorLocked) DrawText("MOUSE UNLOCKED (PRESS TAB)", 10, 60, 20, RED);

        EndDrawing();
    }

    UnloadRenderTexture(target);
    UnloadImage(heightmap);
    UnloadImage(colormap);
    CloseWindow();

    return 0;
}

// ... (Rest of the functions: DrawMessage, GenerateProceduralTerrain, GenerateTerrainPixel, DrawVertexSpace, ApplyFog remain exactly the same)

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

    // Scale 25.0f for huge continents
    heightmap = GenImagePerlinNoise(MAP_SIZE, MAP_SIZE, offsetX, offsetY, NOISE_SCALE);

    ImageFormat(&heightmap, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    heightmapData = (Color*)heightmap.data;

    // 2. Generate Colormap
    colormap = GenImageColor(MAP_SIZE, MAP_SIZE, BLACK);
    ImageFormat(&colormap, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    colormapData = (Color*)colormap.data;

    // 3. Loop to colorize (Interactive Loading Screen)
    for (int y = 0; y < MAP_SIZE; y++)
    {
        for (int x = 0; x < MAP_SIZE; x++)
        {
            int i = y * MAP_SIZE + x;
            GenerateTerrainPixel(&colormapData[i], &heightmapData[i]);
        }
    }
}

void GenerateTerrainPixel(Color *colPixel, Color *hPixel)
{
    unsigned char h = hPixel->r; // Read current height

    // 1. WATER (Flatten logic)
    if (h < 60) {
        hPixel->r = 60;

        if (h < 40) *colPixel = waterDeep;
        else *colPixel = waterShallow;

        return;
    }

    // 2. SAND (Beaches)
    else if (h < 95 + GetRandomValue(-5, 15)) {
        *colPixel = sand;
        colPixel->r += GetRandomValue(-10, 10);
        colPixel->g += GetRandomValue(-10, 10);
    }

    // 3. LOW GRASS
    else if (h < 120 + GetRandomValue(-10, 25)) {
        *colPixel = grassLow;
        colPixel->g += GetRandomValue(-15, 15);
    }

    // 4. HIGH GRASS / FOREST
    else if (h < 140 + GetRandomValue(-20, 50)) {
        *colPixel = grassHigh;
        colPixel->g += GetRandomValue(-25, 10);

        // Add some height noise to make trees look jagged
        hPixel->r += GetRandomValue(0, 10);
    }

    // 5. ROCK
    else if (h < 215 + GetRandomValue(-10, 10)) {
        *colPixel = rock;
        int r = GetRandomValue(-10, 10);
        colPixel->r += r;
        colPixel->g += r;
        colPixel->b += r;
    }

    // 6. SNOW
    else {
        *colPixel = snow;
    }
}

void DrawVertexSpace(void)
{
    // 1. Reset Y-Buffer
    for (int i = 0; i < GAME_WIDTH; i++) {
        y_buffer[i] = GAME_HEIGHT;
    }

    float pleft_x, pleft_y, pright_x, pright_y;

    // 2. Loop FRONT to BACK
    for (int p = 1; p < MAX_PLANES; p++)
    {
        int step = 1 + (p / LOD_FACTOR);

        float fog_factor = 0.0f;
        if (p > fog_start) {
            float normalized_distance = ((float)p - fog_start) / (fog_end - fog_start);
            if (normalized_distance > 1.0f) normalized_distance = 1.0f;
            fog_factor = 1.0f - expf(-normalized_distance * fog_density);
        }

        // Calculate frustum coordinates
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

            // Occlusion Check
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

            int height = heightmapData[index].r;
            int screen_y = (int)((camera_z - height) / (float)p * MAP_Z_SCALE + horizon);

            if (screen_y < lowest_horizon)
            {
                if (screen_y < 0) screen_y = 0;
                int draw_height = lowest_horizon - screen_y;

                if (draw_height > 0)
                {
                    Color col = colormapData[index];
                    col = ApplyFog(col, fog_factor);

                    DrawRectangle(screen_x, screen_y, fill_width, draw_height, col);

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
