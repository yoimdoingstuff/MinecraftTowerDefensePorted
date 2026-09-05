#include "grid.h"
#include <string.h>

void grid_load(Grid *g, const int *tile_data, int width, int height,
               const int *path_xy, int path_len) {
    memset(g, 0, sizeof(*g));
    g->width = width;
    g->height = height;
    for (int y = 0; y < height && y < GRID_MAX_H; y++) {
        for (int x = 0; x < width && x < GRID_MAX_W; x++) {
            g->tiles[y][x] = (TileType)tile_data[y * width + x];
        }
    }
    if (path_len > GRID_MAX_PATH) path_len = GRID_MAX_PATH;
    g->path_len = path_len;
    for (int i = 0; i < path_len; i++) {
        g->path_x[i] = path_xy[i * 2 + 0];
        g->path_y[i] = path_xy[i * 2 + 1];
    }
}

TileType grid_tile_at(const Grid *g, int tx, int ty) {
    if (tx < 0 || ty < 0 || tx >= g->width || ty >= g->height) return TILE_BLOCKED;
    return g->tiles[ty][tx];
}

int grid_is_buildable(const Grid *g, int tx, int ty) {
    return grid_tile_at(g, tx, ty) == TILE_BUILDABLE;
}

int grid_path_adjacent(const Grid *g, int tx, int ty, int dx, int dy) {
    return grid_tile_at(g, tx + dx, ty + dy) == TILE_PATH;
}

void grid_tile_to_pixel(int tx, int ty, int *px, int *py) {
    *px = tx * TILE_SIZE;
    *py = ty * TILE_SIZE;
}

void grid_pixel_to_tile(int px, int py, int *tx, int *ty) {
    *tx = px / TILE_SIZE;
    *ty = py / TILE_SIZE;
}
