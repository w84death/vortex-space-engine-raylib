#ifndef CONSTANTS_H
#define CONSTANTS_H

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080
#define GAME_WIDTH 1280
#define GAME_HEIGHT 720  // 16:9 aspect ratio

#define MAX_PLANES 1782
#define MAP_Z_SCALE 256.0f
#define MOVE_SPEED 180.0f
#define LOD_FACTOR 512
#define CAMERA_MIN_HEIGHT 255
#define CAMERA_CEILING_HEIGHT 1024

// Mouselook Settings
#define MOUSE_SENSITIVITY_X 0.003f
#define MOUSE_SENSITIVITY_Y 2.0f



// Fixed-point math constants
#define FIXED_POINT_SHIFT 16
#define FIXED_POINT_SCALE (1 << FIXED_POINT_SHIFT)

// Terrain Layers (Heightmap values 0-255)
#define LEVEL_WATER 30
#define LEVEL_SAND 45
#define LEVEL_GRASS_LOW 100
#define LEVEL_GRASS_HIGH 140
#define LEVEL_ROCK 200



// DawnBringer-32 Palette
#define DB_BLACK          (Color){   0,   0,   0, 255 }
#define DB_VALHALLA       (Color){  34,  32,  52, 255 }
#define DB_LOULOU         (Color){  69,  40,  60, 255 }
#define DB_OILED_CEDAR    (Color){ 102,  57,  49, 255 }
#define DB_ROPE           (Color){ 143,  86,  59, 255 }
#define DB_TAHITI_GOLD    (Color){ 223, 113,  38, 255 }
#define DB_TWINE          (Color){ 217, 160, 102, 255 }
#define DB_PANCHO         (Color){ 238, 195, 154, 255 }
#define DB_GOLDEN_FIZZ    (Color){ 251, 242,  54, 255 }
#define DB_ATLANTIS       (Color){ 153, 229,  80, 255 }
#define DB_CHRISTI        (Color){ 106, 190,  48, 255 }
#define DB_ELF_GREEN      (Color){  55, 148, 110, 255 }
#define DB_DELL           (Color){  75, 105,  47, 255 }
#define DB_VERDUN_GREEN   (Color){  82,  75,  36, 255 }
#define DB_OPAL           (Color){  50,  60,  57, 255 }
#define DB_DEEP_KOAMARU   (Color){  63,  63, 116, 255 }
#define DB_VENICE_BLUE    (Color){  48,  96, 130, 255 }
#define DB_ROYAL_BLUE     (Color){  91, 110, 225, 255 }
#define DB_KURT           (Color){  99, 155, 255, 255 }
#define DB_WIND_BLUE      (Color){  95, 205, 228, 255 }
#define DB_LINK_WATER     (Color){ 203, 219, 252, 255 }
#define DB_WHITE          (Color){ 255, 255, 255, 255 }
#define DB_SILVER         (Color){ 155, 173, 183, 255 }
#define DB_IRON           (Color){ 132, 126, 135, 255 }
#define DB_SHUTTLE_GREY   (Color){ 105, 106, 106, 255 }
#define DB_CASCADE        (Color){  89,  86,  82, 255 }
#define DB_MING           (Color){ 118,  66, 138, 255 }
#define DB_MOZART         (Color){ 172,  50,  50, 255 }
#define DB_OLD_ROSE       (Color){ 217,  87,  99, 255 }
#define DB_MAUVELOUS      (Color){ 215, 123, 186, 255 }
#define DB_APPLE_BLOSSOM  (Color){ 143, 151,  74, 255 }
#define DB_SAPLING        (Color){ 138, 111,  48, 255 }

#endif // CONSTANTS_H
