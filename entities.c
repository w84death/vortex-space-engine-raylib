#include "entities.h"
#include "constants.h"
#include <stdlib.h>
#include <math.h>

void InitEntityManager(EntityManager *manager) {
    manager->count = 0;
    for(int i=0; i<MAX_ENTITIES; i++) {
        manager->list[i].active = false;
    }
}

void AddEntity(EntityManager *manager, EntityType type, float x, float y) {
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
    
    if (type == ENTITY_SHIP) {
        e->width = 8;
        e->length = 20;
        e->height_shape = 8; 
        e->z_offset = 60;    // Water level
        e->color = (Color){100, 100, 110, 255}; // Ironclad gray
        
        e->speed = (float)GetRandomValue(10, 30); // Random speed
        float angle = (float)GetRandomValue(0, 628) / 100.0f; // 0 to 2PI
        e->dx = cosf(angle) * e->speed;
        e->dy = sinf(angle) * e->speed;
    }
    
    manager->count++;
}

void UpdateEntities(EntityManager *manager, float deltaTime, const Terrain *terrain) {
    for(int i=0; i<MAX_ENTITIES; i++) {
        Entity *e = &manager->list[i];
        if (!e->active) continue;

        if (e->type == ENTITY_SHIP) {
             float nextX = e->x + e->dx * deltaTime;
             float nextY = e->y + e->dy * deltaTime;
             
             // Map bounds wrapping
             if (nextX < 0) nextX += MAP_SIZE;
             if (nextX >= MAP_SIZE) nextX -= MAP_SIZE;
             if (nextY < 0) nextY += MAP_SIZE;
             if (nextY >= MAP_SIZE) nextY -= MAP_SIZE;

             // Check collision with terrain
             int mapX = (int)nextX & (MAP_SIZE - 1);
             int mapY = (int)nextY & (MAP_SIZE - 1);
             int index = mapY * MAP_SIZE + mapX;
             
             // Water level is 60. If terrain is higher, it's land.
             // We allow a small tolerance (e.g. 62) for shorelines
             if (terrain->heightmapRaw[index] > 62) {
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
                int mx = (px + dx) & (MAP_SIZE - 1);
                int my = (py + dy) & (MAP_SIZE - 1);
                int mapIndex = my * MAP_SIZE + mx;
                
                if (bufIndex >= MAX_ENTITY_SIZE * MAX_ENTITY_SIZE) break;

                // 1. Save background
                e->savedHeights[bufIndex] = terrain->heightmapRaw[mapIndex];
                e->savedColors[bufIndex] = terrain->colormapData[mapIndex];

                // 2. Paint Entity
                // Only paint if the entity is "above" the existing terrain
                unsigned char currentH = terrain->heightmapRaw[mapIndex];
                unsigned char entityH = (unsigned char)(e->z_offset + e->height_shape);

                if (entityH >= currentH) {
                    terrain->heightmapRaw[mapIndex] = entityH;
                    
                    // Simple detail: border is darker
                    if (dx == 0 || dx == e->width-1 || dy == 0 || dy == e->length-1) {
                         terrain->colormapData[mapIndex] = (Color){e->color.r/2, e->color.g/2, e->color.b/2, 255};
                    } else {
                         terrain->colormapData[mapIndex] = e->color;
                    }
                    
                    // Draw a little "cabin" box on the ship
                    if (e->type == ENTITY_SHIP) {
                        if (dx >= 2 && dx <= e->width-3 && dy >= 2 && dy <= 6) {
                            terrain->heightmapRaw[mapIndex] += 4; // Taller cabin
                            terrain->colormapData[mapIndex] = RAYWHITE;
                        }
                    }
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
                int mx = (e->paint_x + dx) & (MAP_SIZE - 1);
                int my = (e->paint_y + dy) & (MAP_SIZE - 1);
                int mapIndex = my * MAP_SIZE + mx;
                
                if (bufIndex >= MAX_ENTITY_SIZE * MAX_ENTITY_SIZE) break;

                terrain->heightmapRaw[mapIndex] = e->savedHeights[bufIndex];
                terrain->colormapData[mapIndex] = e->savedColors[bufIndex];
                
                bufIndex++;
            }
        }
    }
}