#ifndef SETTINGS_H
#define SETTINGS_H

typedef enum {
    MODE_GAME,
    MODE_EDITOR,
    MODE_QUIT
} GameMode;

typedef struct {
    GameMode gameMode;
    int mapSize;
    float noiseScale;
    int shipCount;
    int unitCount;
    int buildingCount;
} GameSettings;

extern GameSettings gameSettings;

#endif // SETTINGS_H