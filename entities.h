#ifndef ENTITIES_H
#define ENTITIES_H

#include "raylib.h"
#include "terrain.h"

typedef enum {
    ENTITY_SHIP,
    ENTITY_UNIT,
    ENTITY_BUILDING
} EntityType;

typedef struct {
    char name[64];
    EntityType type;
    int width;
    int length;
    unsigned char heights[MAX_ENTITY_SIZE * MAX_ENTITY_SIZE];
    Color colors[MAX_ENTITY_SIZE * MAX_ENTITY_SIZE];
} VoxelModel;

#define MAX_LOADED_MODELS 128

typedef struct {
    VoxelModel models[MAX_LOADED_MODELS];
    int count;
} ModelRegistry;

extern ModelRegistry modelRegistry;

typedef struct {
    bool active;
    EntityType type;
    const VoxelModel *model;

    // Position and Movement
    float x, y, z;     // Map coordinates. z is usually base height + offset
    float dx, dy;      // Velocity
    float angle;       // Rotation in radians
    float speed;

    // Dimensions
    int width;
    int length;
    int height_shape;  // How tall the voxel shape is
    int z_offset;

    // Visuals
    Color color;
    unsigned char savedHeights[MAX_ENTITY_SIZE * MAX_ENTITY_SIZE];
    Color savedColors[MAX_ENTITY_SIZE * MAX_ENTITY_SIZE];
    int paint_x, paint_y, paint_w, paint_h;
} Entity;

typedef struct {
    Entity list[MAX_ENTITIES];
    int count;
} EntityManager;

void InitEntityManager(EntityManager *manager);
void AddEntity(EntityManager *manager, EntityType type, float x, float y);
void AddEntityFromModel(EntityManager *manager, EntityType type, float x, float y, const VoxelModel *model);
void UpdateEntities(EntityManager *manager, float deltaTime, const Terrain *terrain);

// The "Paint & Restore" Rendering methods
void PaintEntities(EntityManager *manager, Terrain *terrain);
void RestoreEntities(EntityManager *manager, Terrain *terrain);

// Model Management
void InitModelRegistry();
void LoadAllModels();
void SaveModel(const VoxelModel *model);
VoxelModel* GetRandomModel(EntityType type);

#endif // ENTITIES_H
