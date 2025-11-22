#ifndef CONSTANTS_H
#define CONSTANTS_H

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

// Fixed-point math constants
#define FIXED_POINT_SHIFT 16
#define FIXED_POINT_SCALE (1 << FIXED_POINT_SHIFT)

// Terrain Layers (Heightmap values 0-255)
#define LEVEL_WATER 60
#define LEVEL_SAND 95
#define LEVEL_GRASS_LOW 120
#define LEVEL_GRASS_HIGH 140
#define LEVEL_ROCK 215

#endif // CONSTANTS_H
