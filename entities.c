#include "entities.h"
#include "settings.h"
#include "constants.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

ModelRegistry modelRegistry = {0};

void InitModelRegistry() {
    modelRegistry.count = 0;
}

void LoadModelFromFile(const char* filepath, EntityType type) {
    if (modelRegistry.count >= MAX_LOADED_MODELS) return;
    
    FILE *f = fopen(filepath, "r");
    if (!f) return;
    
    VoxelModel *m = &modelRegistry.models[modelRegistry.count];
    
    // Clear model first
    for(int i=0; i<MAX_ENTITY_SIZE*MAX_ENTITY_SIZE; i++) {
        m->heights[i] = 0;
        m->colors[i] = BLANK;
    }

    if (fscanf(f, "%d %d", &m->width, &m->length) != 2) { fclose(f); return; }
    
    if (m->width > MAX_ENTITY_SIZE) m->width = MAX_ENTITY_SIZE;
    if (m->length > MAX_ENTITY_SIZE) m->length = MAX_ENTITY_SIZE;
    
    for(int y=0; y < m->length; y++) {
        for(int x=0; x < m->width; x++) {
            int h, r, g, b, a;
            if (fscanf(f, "%d %d %d %d %d", &h, &r, &g, &b, &a) == 5) {
                int idx = y * MAX_ENTITY_SIZE + x;
                m->heights[idx] = (unsigned char)h;
                m->colors[idx] = (Color){r,g,b,a};
            }
        }
    }
    
    // Extract name
    const char* base = strrchr(filepath, '/');
    if (!base) base = filepath; else base++;
    strncpy(m->name, base, 63);
    char* ext = strrchr(m->name, '.');
    if (ext) *ext = 0;
    
    m->type = type;
    
    fclose(f);
    modelRegistry.count++;
    TraceLog(LOG_INFO, "Loaded model: %s", m->name);
}

void LoadModelsFromDir(const char* dirname, EntityType type) {
    DIR *d;
    struct dirent *dir;
    d = opendir(dirname);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_name[0] == '.') continue;
            char path[256];
            snprintf(path, sizeof(path), "%s/%s", dirname, dir->d_name);
            LoadModelFromFile(path, type);
        }
        closedir(d);
    }
}

void LoadAllModels() {
    InitModelRegistry();
    LoadModelsFromDir("models/ship", ENTITY_SHIP);
    LoadModelsFromDir("models/unit", ENTITY_UNIT);
    LoadModelsFromDir("models/building", ENTITY_BUILDING);
}

void SaveModel(const VoxelModel *model) {
    char path[256];
    const char* subfolder = "unit";
    if (model->type == ENTITY_SHIP) subfolder = "ship";
    if (model->type == ENTITY_BUILDING) subfolder = "building";
    
    // Use a default name if empty or generic
    const char* name = (strlen(model->name) > 0) ? model->name : "unnamed";
    
    snprintf(path, sizeof(path), "models/%s/%s.txt", subfolder, name);
    
    FILE *f = fopen(path, "w");
    if (!f) return;
    
    fprintf(f, "%d %d\n", model->width, model->length);
    for(int y=0; y < model->length; y++) {
        for(int x=0; x < model->width; x++) {
            int idx = y * MAX_ENTITY_SIZE + x;
            unsigned char h = model->heights[idx];
            Color c = model->colors[idx];
            fprintf(f, "%d %d %d %d %d\n", h, c.r, c.g, c.b, c.a);
        }
    }
    fclose(f);
    TraceLog(LOG_INFO, "Saved model to %s", path);
}

VoxelModel* GetRandomModel(EntityType type) {
    int indices[MAX_LOADED_MODELS];
    int count = 0;
    for(int i=0; i<modelRegistry.count; i++) {
        if (modelRegistry.models[i].type == type) {
            indices[count++] = i;
        }
    }
    
    if (count > 0) {
        return &modelRegistry.models[indices[GetRandomValue(0, count-1)]];
    }
    return NULL;
}

void InitEntityManager(EntityManager *manager) {
    LoadAllModels(); // Load models on init
    manager->count = 0;
    for(int i=0; i<MAX_ENTITIES; i++) {
        manager->list[i].active = false;
    }
}

void AddEntityFromModel(EntityManager *manager, EntityType type, float x, float y, const VoxelModel *model) {
    int slot = -1;
    // Find first inactive slot
    for(int i=0; i<MAX_ENTITIES; i++) {
        if(!manager->list[i].active) {
            slot = i;
            break;
        }
    }
    
    // If no free slot, fail safely
    if (slot == -1) return;

    Entity *e = &manager->list[slot];
    e->active = true;
    e->type = type;
    e->x = x;
    e->y = y;
    e->model = model;

    if (e->model) {
        e->width = model->width;
        e->length = model->length;
        e->height_shape = 0; // Driven by model
    }

    if (type == ENTITY_SHIP) {
        if (!e->model) {
            e->width = 8;
            e->length = 20;
            e->height_shape = 8; 
            e->color = (Color){100, 100, 110, 255};
        }
        e->z_offset = LEVEL_WATER;
        
        e->speed = (float)GetRandomValue(10, 30); // Random speed
        float angle = (float)GetRandomValue(0, 628) / 100.0f; // 0 to 2PI
        e->dx = cosf(angle) * e->speed;
        e->dy = sinf(angle) * e->speed;
    }
    else if (type == ENTITY_UNIT) {
        if (!e->model) {
            e->width = 2;
            e->length = 2;
            e->height_shape = 4;
            e->color = BLACK;
        }
        e->z_offset = 0;
        
        e->speed = (float)GetRandomValue(20, 40);
        float angle = (float)GetRandomValue(0, 628) / 100.0f;
        e->dx = cosf(angle) * e->speed;
        e->dy = sinf(angle) * e->speed;
    }
    else if (type == ENTITY_BUILDING) {
        if (!e->model) {
            e->width = 8;
            e->length = 8;
            e->height_shape = 8;
            e->color = BROWN;
        }
        e->z_offset = 0;
        
        e->speed = 0;
        e->dx = 0;
        e->dy = 0;
    }
    
    manager->count++;
}

void AddEntity(EntityManager *manager, EntityType type, float x, float y) {
    AddEntityFromModel(manager, type, x, y, GetRandomModel(type));
}

void UpdateEntities(EntityManager *manager, float deltaTime, const Terrain *terrain) {
    for(int i=0; i<MAX_ENTITIES; i++) {
        Entity *e = &manager->list[i];
        if (!e->active) continue;

        if (e->type == ENTITY_SHIP) {
             float nextX = e->x + e->dx * deltaTime;
             float nextY = e->y + e->dy * deltaTime;
             
             // Map bounds wrapping
             if (nextX < 0) nextX += gameSettings.mapSize;
             if (nextX >= gameSettings.mapSize) nextX -= gameSettings.mapSize;
             if (nextY < 0) nextY += gameSettings.mapSize;
             if (nextY >= gameSettings.mapSize) nextY -= gameSettings.mapSize;

             // Check collision with terrain
             int mapX = (int)nextX & (gameSettings.mapSize - 1);
             int mapY = (int)nextY & (gameSettings.mapSize - 1);
             int index = mapY * gameSettings.mapSize + mapX;
             
             // Water level is LEVEL_WATER. If terrain is higher, it's land.
             // We allow a small tolerance for shorelines
             if (terrain->heightmapRaw[index] > LEVEL_WATER + 2) {
                 // Bounce: simplistic reflection
                 e->dx = -e->dx;
                 e->dy = -e->dy;
                 
                 // Add some randomness
                 float noise = ((float)GetRandomValue(-100, 100) / 100.0f) * 0.5f; 
                 e->dx += noise;
                 e->dy += noise;
             } else {
                 e->x = nextX;
                 e->y = nextY;
             }
        }
        else if (e->type == ENTITY_UNIT) {
             float nextX = e->x + e->dx * deltaTime;
             float nextY = e->y + e->dy * deltaTime;
             
             // Map bounds wrapping
             if (nextX < 0) nextX += gameSettings.mapSize;
             if (nextX >= gameSettings.mapSize) nextX -= gameSettings.mapSize;
             if (nextY < 0) nextY += gameSettings.mapSize;
             if (nextY >= gameSettings.mapSize) nextY -= gameSettings.mapSize;

             // Check collision with terrain
             int mapX = (int)nextX & (gameSettings.mapSize - 1);
             int mapY = (int)nextY & (gameSettings.mapSize - 1);
             int index = mapY * gameSettings.mapSize + mapX;
             
             // Land units stay on land. Bounce on water.
             if (terrain->heightmapRaw[index] <= LEVEL_WATER + 2) {
                 e->dx = -e->dx;
                 e->dy = -e->dy;
                 
                 float noise = ((float)GetRandomValue(-100, 100) / 100.0f) * 0.5f; 
                 e->dx += noise;
                 e->dy += noise;
             } else {
                 e->x = nextX;
                 e->y = nextY;
             }
        }
    }
}

void PaintEntities(EntityManager *manager, Terrain *terrain) {
    for(int i=0; i<MAX_ENTITIES; i++) {
        Entity *e = &manager->list[i];
        if (!e->active) continue;

        int px = (int)e->x - e->width/2;
        int py = (int)e->y - e->length/2;
        
        // Store bounds for Restore pass
        e->paint_x = px;
        e->paint_y = py;
        e->paint_w = e->width;
        e->paint_h = e->length;

        int bufIndex = 0;
        for(int dy = 0; dy < e->length; dy++) {
            for(int dx = 0; dx < e->width; dx++) {
                // Handle map wrapping
                int mx = (px + dx) & (gameSettings.mapSize - 1);
                int my = (py + dy) & (gameSettings.mapSize - 1);
                int mapIndex = my * gameSettings.mapSize + mx;
                
                if (bufIndex >= MAX_ENTITY_SIZE * MAX_ENTITY_SIZE) break;

                // 1. Save background
                e->savedHeights[bufIndex] = terrain->heightmapRaw[mapIndex];
                e->savedColors[bufIndex] = terrain->colormapData[mapIndex];

                // 2. Paint Entity
                // Only paint if the entity is "above" the existing terrain
                unsigned char currentH = terrain->heightmapRaw[mapIndex];
                
                unsigned char baseH = (e->type == ENTITY_SHIP) ? (unsigned char)e->z_offset : currentH;
                unsigned char entityH = 0;
                Color entityC = BLANK;
                bool draw = false;

                if (e->model) {
                    int idx = dy * MAX_ENTITY_SIZE + dx;
                    unsigned char h = e->model->heights[idx];
                    if (h > 0) {
                        entityH = baseH + h;
                        entityC = e->model->colors[idx];
                        draw = true;
                    }
                } else {
                    // Procedural Fallback
                    entityH = baseH + (unsigned char)e->height_shape;
                    if (dx == 0 || dx == e->width-1 || dy == 0 || dy == e->length-1) {
                         entityC = (Color){e->color.r/2, e->color.g/2, e->color.b/2, 255};
                    } else {
                         entityC = e->color;
                    }
                    
                    // Procedural details
                    if (e->type == ENTITY_SHIP) {
                        if (dx >= 2 && dx <= e->width-3 && dy >= 2 && dy <= 6) {
                            entityH += 4;
                            entityC = RAYWHITE;
                        }
                    }
                    else if (e->type == ENTITY_BUILDING) {
                        int cx = e->width / 2;
                        int cy = e->length / 2;
                        int distX = abs(dx - cx);
                        int distY = abs(dy - cy);
                        int dist = (distX > distY) ? distX : distY;
                        int roofHeight = (cx - dist) * 2;
                        if (roofHeight > 0) {
                            entityH += roofHeight;
                            entityC = (Color){160, 82, 45, 255};
                        }
                    }
                    draw = true;
                }

                if (draw && entityH >= currentH) {
                    terrain->heightmapRaw[mapIndex] = entityH;
                    terrain->colormapData[mapIndex] = entityC;
                }

                bufIndex++;
            }
        }
    }
}

void RestoreEntities(EntityManager *manager, Terrain *terrain) {
    // LIFO Restore to handle overlaps correctly
    for(int i = MAX_ENTITIES - 1; i >= 0; i--) {
        Entity *e = &manager->list[i];
        if (!e->active) continue;

        int bufIndex = 0;
        for(int dy = 0; dy < e->paint_h; dy++) {
            for(int dx = 0; dx < e->paint_w; dx++) {
                int mx = (e->paint_x + dx) & (gameSettings.mapSize - 1);
                int my = (e->paint_y + dy) & (gameSettings.mapSize - 1);
                int mapIndex = my * gameSettings.mapSize + mx;
                
                if (bufIndex >= MAX_ENTITY_SIZE * MAX_ENTITY_SIZE) break;

                terrain->heightmapRaw[mapIndex] = e->savedHeights[bufIndex];
                terrain->colormapData[mapIndex] = e->savedColors[bufIndex];
                
                bufIndex++;
            }
        }
    }
}