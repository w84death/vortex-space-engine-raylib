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

// Terrain Colors
#define COLOR_DEEP_OCEAN    (Color){ 20, 45, 75, 255 }
#define COLOR_SHALLOW_WATER (Color){ 40, 80, 90, 255 }
#define COLOR_BEACH_SAND    (Color){ 215, 205, 185, 255 }
#define COLOR_WET_SAND      (Color){ 190, 175, 150, 255 }
#define COLOR_GRASS         (Color){ 115, 140, 70, 255 }
#define COLOR_TREES         (Color){ 40, 80, 45, 255 }
#define COLOR_ROCK          (Color){ 90, 90, 90, 255 }
#define COLOR_SNOW          (Color){ 255, 255, 255, 255 }

#endif // CONSTANTS_H
