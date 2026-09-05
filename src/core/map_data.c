#include "map_data.h"

/* ---- forest: horizontal serpentine, 3 full path rows ---- */
#define B TILE_BUILDABLE
#define P TILE_PATH
static const int FOREST_TILES[6][12] = {
    {P,P,P,P,P,P,P,P,P,P,P,P},
    {B,B,B,B,B,B,B,B,B,B,B,P},
    {P,P,P,P,P,P,P,P,P,P,P,P},
    {P,B,B,B,B,B,B,B,B,B,B,B},
    {P,P,P,P,P,P,P,P,P,P,P,P},
    {B,B,B,B,B,B,B,B,B,B,B,P},
};
static const int FOREST_PATH[] = {
    0,0, 1,0, 2,0, 3,0, 4,0, 5,0, 6,0, 7,0, 8,0, 9,0, 10,0, 11,0,
    11,1,
    11,2, 10,2, 9,2, 8,2, 7,2, 6,2, 5,2, 4,2, 3,2, 2,2, 1,2, 0,2,
    0,3,
    0,4, 1,4, 2,4, 3,4, 4,4, 5,4, 6,4, 7,4, 8,4, 9,4, 10,4, 11,4,
    11,5,
};

/* ---- village: vertical serpentine through 3 columns ---- */
static const int VILLAGE_TILES[6][12] = {
    {B,B,P,B,B,B,P,P,P,P,P,B},
    {B,B,P,B,B,B,P,B,B,B,P,B},
    {B,B,P,B,B,B,P,B,B,B,P,B},
    {B,B,P,B,B,B,P,B,B,B,P,B},
    {B,B,P,B,B,B,P,B,B,B,P,B},
    {B,B,P,P,P,P,P,B,B,B,P,B},
};
static const int VILLAGE_PATH[] = {
    2,0, 2,1, 2,2, 2,3, 2,4, 2,5,
    3,5, 4,5, 5,5, 6,5,
    6,4, 6,3, 6,2, 6,1, 6,0,
    7,0, 8,0, 9,0, 10,0,
    10,1, 10,2, 10,3, 10,4, 10,5,
};

/* ---- ravine: diagonal staircase, top-left to bottom-right ---- */
static const int RAVINE_TILES[6][12] = {
    {P,P,P,B,B,B,B,B,B,B,B,B},
    {B,B,P,P,P,B,B,B,B,B,B,B},
    {B,B,B,B,P,P,P,B,B,B,B,B},
    {B,B,B,B,B,B,P,P,P,B,B,B},
    {B,B,B,B,B,B,B,B,P,P,P,B},
    {B,B,B,B,B,B,B,B,B,B,P,P},
};
static const int RAVINE_PATH[] = {
    0,0, 1,0, 2,0,
    2,1, 3,1, 4,1,
    4,2, 5,2, 6,2,
    6,3, 7,3, 8,3,
    8,4, 9,4, 10,4,
    10,5, 11,5,
};
#undef B
#undef P

const MapDef MAP_DEFS[MAP_COUNT] = {
    {
        .display_name = "Forest",
        .background_asset = "images/map_forest_bg.png",
        .width = 12, .height = 6,
        .tiles = (const int *)FOREST_TILES,
        .path_xy = FOREST_PATH,
        .path_len = sizeof(FOREST_PATH) / sizeof(int) / 2,
        .total_waves = 15,
    },
    {
        .display_name = "Village",
        .background_asset = "images/map_village_bg.png",
        .width = 12, .height = 6,
        .tiles = (const int *)VILLAGE_TILES,
        .path_xy = VILLAGE_PATH,
        .path_len = sizeof(VILLAGE_PATH) / sizeof(int) / 2,
        .total_waves = 15,
    },
    {
        .display_name = "Ravine",
        .background_asset = "images/map_ravine_bg.png",
        .width = 12, .height = 6,
        .tiles = (const int *)RAVINE_TILES,
        .path_xy = RAVINE_PATH,
        .path_len = sizeof(RAVINE_PATH) / sizeof(int) / 2,
        .total_waves = 15,
    },
};

void map_load_into_grid(int map_index, Grid *out_grid) {
    if (map_index < 0 || map_index >= MAP_COUNT) map_index = 0;
    const MapDef *m = &MAP_DEFS[map_index];
    grid_load(out_grid, m->tiles, m->width, m->height, m->path_xy, m->path_len);
}
