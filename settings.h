#ifndef SETTINGS_H
#define SETTINGS_H

typedef struct {
    int mapSize;
    float noiseScale;
    int shipCount;
    int unitCount;
    int buildingCount;
} GameSettings;

extern GameSettings gameSettings;

#endif // SETTINGS_H