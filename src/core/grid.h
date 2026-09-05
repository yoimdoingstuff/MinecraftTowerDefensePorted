/* Tile grid.
 *
 * Reverse-engineered rule (see docs/original-game-reference.md): each
 * map is a fixed grid of 40x40px tiles. Tiles are either a buildable
 * plot (towers/traps can be placed there) or part of a single fixed
 * lane that enemies walk from spawn to exit. There is no dynamic
 * pathfinding and no player-built maze - the lane shape is baked into
 * the map, matching the pre-carved corridor visible in the original
 * game's own map art.
 */
#ifndef MTD_GRID_H
#define MTD_GRID_H

#define TILE_SIZE 40
#define GRID_MAX_W 12
#define GRID_MAX_H 6
#define GRID_MAX_PATH (GRID_MAX_W * GRID_MAX_H)

typedef enum {
    TILE_BUILDABLE = 0,
    TILE_PATH = 1,
    TILE_BLOCKED = 2  /* decorative/unusable, e.g. map border */
} TileType;

typedef struct {
    int width, height;         /* in tiles */
    TileType tiles[GRID_MAX_H][GRID_MAX_W];

    /* Ordered waypoints (tile coords) enemies walk through, spawn to exit. */
    int path_x[GRID_MAX_PATH];
    int path_y[GRID_MAX_PATH];
    int path_len;
} Grid;

void grid_load(Grid *g, const int *tile_data, int width, int height,
               const int *path_xy, int path_len);

/* -1 if out of bounds */
TileType grid_tile_at(const Grid *g, int tx, int ty);

/* True if (tx,ty) is in bounds and currently free to build on (not
 * already occupied - occupancy is tracked by the tower list, so this
 * only checks tile type; game.c checks occupancy separately). */
int grid_is_buildable(const Grid *g, int tx, int ty);

/* True if a path tile is orthogonally adjacent to (tx,ty). Used by
 * towers to decide which of the 4 directions they can fire in. */
int grid_path_adjacent(const Grid *g, int tx, int ty, int dx, int dy);

void grid_tile_to_pixel(int tx, int ty, int *px, int *py);
void grid_pixel_to_tile(int px, int py, int *tx, int *ty);

#endif /* MTD_GRID_H */
