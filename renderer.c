#include "renderer.h"
#include "settings.h"
#include <stdlib.h>
#include <math.h>

void InitRenderer(Renderer *renderer) {
  renderer->frameBuffer = (Color*)malloc(GAME_WIDTH * GAME_HEIGHT * sizeof(Color));

  Image blank = GenImageColor(GAME_WIDTH, GAME_HEIGHT, BLACK);
  renderer->screenTexture = LoadTextureFromImage(blank);
  UnloadImage(blank);

  SetTextureFilter(renderer->screenTexture, TEXTURE_FILTER_POINT);

  for(int i=1; i<MAX_PLANES; i++){
    renderer->depth_scale_table[i] = MAP_Z_SCALE / (float)i;
  }

  renderer->sky_color = DB_WIND_BLUE;
}

void ClearFrameBuffer(Renderer *renderer, Color color) {
  for (int i = 0; i < GAME_WIDTH * GAME_HEIGHT; i++) {
    renderer->frameBuffer[i] = color;
  }
}

void DrawVertexSpace(Renderer *renderer, const EngineState *state, const Terrain *terrain) {
  for (int i = 0; i < GAME_WIDTH; i++) {
    renderer->y_buffer[i] = GAME_HEIGHT;
  }

  float pleft_x, pleft_y, pright_x, pright_y;

  for (int p = 1; p < MAX_PLANES; p++)
  {
    int step = 1 + (p / LOD_FACTOR);

    pleft_x = (-state->cosphi * p - state->sinphi * p) + state->camera_x;
    pleft_y = (state->sinphi * p - state->cosphi * p) + state->camera_y;
    pright_x = (state->cosphi * p - state->sinphi * p) + state->camera_x;
    pright_y = (-state->sinphi * p - state->cosphi * p) + state->camera_y;

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

      int lowest_horizon = renderer->y_buffer[screen_x];
      if (step > 1) {
          for (int k = 1; k < fill_width; k++) {
              if (renderer->y_buffer[screen_x + k] > lowest_horizon) {
                  lowest_horizon = renderer->y_buffer[screen_x + k];
              }
          }
      }

      if (lowest_horizon <= 0) {
          cur_map_x_fixed += map_dx_fixed;
          cur_map_y_fixed += map_dy_fixed;
          continue;
      }

      int map_x_int = (cur_map_x_fixed >> FIXED_POINT_SHIFT) & (gameSettings.mapSize - 1);
      int map_y_int = (cur_map_y_fixed >> FIXED_POINT_SHIFT) & (gameSettings.mapSize - 1);
      int index = map_y_int * gameSettings.mapSize + map_x_int;

      int height = terrain->heightmapRaw[index];
      int screen_y = (int)((state->camera_z - height) * renderer->depth_scale_table[p] + state->horizon);

      if (screen_y < lowest_horizon){
        if (screen_y < 0) screen_y = 0;
        int draw_height = lowest_horizon - screen_y;

        if (draw_height > 0){
          Color col = terrain->colormapData[index];

          int base_offset = screen_y * GAME_WIDTH + screen_x;
          for (int k = 0; k < fill_width; k++) {
            int offset = base_offset + k;
            for (int y = screen_y; y < lowest_horizon; y++) {
              renderer->frameBuffer[offset] = col;
              offset += GAME_WIDTH;
            }
          }

          for (int k = 0; k < fill_width; k++) {
            renderer->y_buffer[screen_x + k] = screen_y;
          }
        }
      }

      cur_map_x_fixed += map_dx_fixed;
      cur_map_y_fixed += map_dy_fixed;
    }
  }
}

void UpdateRendererTexture(Renderer *renderer) {
  UpdateTexture(renderer->screenTexture, renderer->frameBuffer);
}

void DrawRendererTextureToScreen(Renderer *renderer) {
  ClearBackground(BLACK);
  Rectangle srcRect = { 0.0f, 0.0f, (float)GAME_WIDTH, (float)GAME_HEIGHT };
  Rectangle destRect = { 0.0f, 0.0f, (float)GetScreenWidth(), (float)GetScreenHeight() };
  DrawTexturePro(renderer->screenTexture, srcRect, destRect, (Vector2){ 0, 0 }, 0.0f, WHITE);
}

bool GetMapCoordinates(const Renderer *renderer, const EngineState *state, const Terrain *terrain, int screenX, int screenY, int *outMapX, int *outMapY) {
  float scaleX = (float)GAME_WIDTH / GetScreenWidth();
  float scaleY = (float)GAME_HEIGHT / GetScreenHeight();

  int gameX = (int)(screenX * scaleX);
  int gameY = (int)(screenY * scaleY);

  if (gameX < 0 || gameX >= GAME_WIDTH || gameY < 0 || gameY >= GAME_HEIGHT) return false;

  float pleft_rel_x = -state->cosphi - state->sinphi;
  float pleft_rel_y = state->sinphi - state->cosphi;

  float pright_rel_x = state->cosphi - state->sinphi;
  float pright_rel_y = -state->sinphi - state->cosphi;

  float dx = (pright_rel_x - pleft_rel_x) / GAME_WIDTH;
  float dy = (pright_rel_y - pleft_rel_y) / GAME_WIDTH;

  float ray_dx = pleft_rel_x + dx * gameX;
  float ray_dy = pleft_rel_y + dy * gameX;

  int lowest_horizon = GAME_HEIGHT;

  for (int p = 1; p < MAX_PLANES; p++) {
    float mapX = state->camera_x + ray_dx * p;
    float mapY = state->camera_y + ray_dy * p;

    int map_x_int = ((int)mapX) & (gameSettings.mapSize - 1);
    int map_y_int = ((int)mapY) & (gameSettings.mapSize - 1);
    int index = map_y_int * gameSettings.mapSize + map_x_int;

    int height = terrain->heightmapRaw[index];

    int projected_y = (int)((state->camera_z - height) * renderer->depth_scale_table[p] + state->horizon);

    if (projected_y < lowest_horizon) {
      if (gameY >= projected_y && gameY <= lowest_horizon) {
        *outMapX = map_x_int;
        *outMapY = map_y_int;
        return true;
      }
      lowest_horizon = projected_y;
    }

    if (lowest_horizon < 0) break;
  }

  return false;
}

void CloseRenderer(Renderer *renderer) {
  free(renderer->frameBuffer);
  UnloadTexture(renderer->screenTexture);
}
