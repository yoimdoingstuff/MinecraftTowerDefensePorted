#include "map_data.h"

/* Tile arrays below are 0=buildable, 1=path. Each one (and its
 * matching _PATH waypoint list) was generated from the waypoint list
 * alone via tools' map generator, not hand-transcribed - see the
 * header comment for why that matters. */

static const int FOREST_TILES[6][12] = {
    {1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0},
    {1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,0,0,0,0,0,0,0,0,0,1},
};
static const int FOREST_PATH[] = {
    0,0, 1,0, 2,0, 3,0, 4,0, 5,0, 6,0, 7,0, 8,0, 9,0, 10,0, 11,0,
    11,1,
    11,2, 10,2, 9,2, 8,2, 7,2, 6,2, 5,2, 4,2, 3,2, 2,2, 1,2, 0,2,
    0,3,
    0,4, 1,4, 2,4, 3,4, 4,4, 5,4, 6,4, 7,4, 8,4, 9,4, 10,4, 11,4,
    11,5,
};

static const int VILLAGE_TILES[6][12] = {
    {0,0,1,0,0,0,1,1,1,1,1,0},
    {0,0,1,0,0,0,1,0,0,0,1,0},
    {0,0,1,0,0,0,1,0,0,0,1,0},
    {0,0,1,0,0,0,1,0,0,0,1,0},
    {0,0,1,0,0,0,1,0,0,0,1,0},
    {0,0,1,1,1,1,1,0,0,0,1,0},
};
static const int VILLAGE_PATH[] = {
    2,0, 2,1, 2,2, 2,3, 2,4, 2,5, 3,5, 4,5, 5,5, 6,5,
    6,4, 6,3, 6,2, 6,1, 6,0, 7,0, 8,0, 9,0, 10,0,
    10,1, 10,2, 10,3, 10,4, 10,5,
};

static const int RAVINE_TILES[6][12] = {
    {1,1,1,0,0,0,0,0,0,0,0,0},
    {0,0,1,1,1,0,0,0,0,0,0,0},
    {0,0,0,0,1,1,1,0,0,0,0,0},
    {0,0,0,0,0,0,1,1,1,0,0,0},
    {0,0,0,0,0,0,0,0,1,1,1,0},
    {0,0,0,0,0,0,0,0,0,0,1,1},
};
static const int RAVINE_PATH[] = {
    0,0, 1,0, 2,0, 2,1, 3,1, 4,1, 4,2, 5,2, 6,2,
    6,3, 7,3, 8,3, 8,4, 9,4, 10,4, 10,5, 11,5,
};

static const int ICE_PEAK_TILES[6][12] = {
    {0,0,0,0,0,1,1,1,1,1,1,1},
    {0,0,0,0,1,1,0,0,0,0,0,0},
    {0,0,0,1,1,0,0,0,0,0,0,0},
    {0,0,1,1,0,0,0,0,0,0,0,0},
    {0,1,1,0,0,0,0,0,0,0,0,0},
    {1,1,0,0,0,0,0,0,0,0,0,0},
};
static const int ICE_PEAK_PATH[] = {
    0,5, 1,5, 1,4, 2,4, 2,3, 3,3, 3,2, 4,2, 4,1, 5,1, 5,0,
    6,0, 7,0, 8,0, 9,0, 10,0, 11,0,
};

static const int ABANDONED_MINE_TILES[6][12] = {
    {0,0,0,0,0,0,0,0,0,0,0,0},
    {1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0},
};
static const int ABANDONED_MINE_PATH[] = {
    0,1, 1,1, 2,1, 3,1, 4,1, 5,1, 6,1, 7,1, 8,1, 9,1, 10,1, 11,1,
    11,2,
    11,3, 10,3, 9,3, 8,3, 7,3, 6,3, 5,3, 4,3, 3,3, 2,3, 1,3, 0,3,
};

static const int SPIDER_CAVERN_TILES[6][12] = {
    {1,1,0,0,0,0,0,0,0,0,1,1},
    {0,1,1,0,0,0,0,0,0,1,1,0},
    {0,0,1,1,0,0,0,0,1,1,0,0},
    {0,0,0,1,1,0,0,1,1,0,0,0},
    {0,0,0,0,1,1,1,1,0,0,0,0},
    {0,0,0,0,0,1,1,0,0,0,0,0},
};
static const int SPIDER_CAVERN_PATH[] = {
    0,0, 1,0, 1,1, 2,1, 2,2, 3,2, 3,3, 4,3, 4,4, 5,4, 5,5,
    6,5, 6,4, 7,4, 7,3, 8,3, 8,2, 9,2, 9,1, 10,1, 10,0, 11,0,
};

static const int DESERTED_BEACH_TILES[6][12] = {
    {0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,1,1,1,0,0,0,1,1,1,0},
    {1,1,1,0,1,1,1,1,1,0,1,1},
    {0,0,0,0,0,1,1,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0},
};
static const int DESERTED_BEACH_PATH[] = {
    0,3, 1,3, 2,3, 2,2, 3,2, 4,2, 4,3, 5,3, 5,4, 6,4,
    6,3, 7,3, 8,3, 8,2, 9,2, 10,2, 10,3, 11,3,
};

static const int MOUNTAIN_ASCENT_TILES[6][12] = {
    {0,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,0,1,1,0,0,0,0,0,0,0},
    {1,1,0,0,0,0,0,0,0,0,0,0},
    {1,1,0,0,0,0,0,0,0,0,0,0},
    {1,1,0,0,0,0,0,0,0,0,0,0},
    {1,1,0,0,0,0,0,0,0,0,0,0},
};
static const int MOUNTAIN_ASCENT_PATH[] = {
    0,5, 1,5, 1,4, 0,4, 0,3, 1,3, 1,2, 0,2, 0,1, 1,1, 1,0,
    2,0, 3,0, 3,1, 4,1, 4,0, 5,0, 6,0, 7,0, 8,0, 9,0, 10,0, 11,0,
};

static const int NETHER_STRONGHOLD_TILES[6][12] = {
    {1,0,1,1,1,0,0,0,0,0,0,0},
    {1,1,1,0,1,1,0,1,1,1,0,0},
    {0,0,0,0,1,1,1,1,0,1,0,0},
    {0,0,0,0,1,1,1,0,0,1,1,1},
    {0,0,0,0,0,0,0,0,0,0,0,1},
    {0,0,0,0,0,0,0,0,0,0,0,1},
};
static const int NETHER_STRONGHOLD_PATH[] = {
    0,0, 0,1, 1,1, 2,1, 2,0, 3,0, 4,0, 4,1, 5,1, 5,2, 4,2,
    4,3, 5,3, 6,3, 6,2, 7,2, 7,1, 8,1, 9,1, 9,2, 9,3, 10,3,
    11,3, 11,4, 11,5,
};

#define NO_BOSS ENEMY_ZOMBIE, 0

const MapDef MAP_DEFS[MAP_COUNT] = {
    {
        .display_name = "Forest", .tile_tint_r = 255, .tile_tint_g = 255, .tile_tint_b = 255,
        .width = 12, .height = 6, .tiles = (const int *)FOREST_TILES,
        .path_xy = FOREST_PATH, .path_len = sizeof(FOREST_PATH) / sizeof(int) / 2,
        .total_waves = 15, .boss_kind = NO_BOSS,
    },
    {
        .display_name = "Village", .tile_tint_r = 255, .tile_tint_g = 255, .tile_tint_b = 255,
        .width = 12, .height = 6, .tiles = (const int *)VILLAGE_TILES,
        .path_xy = VILLAGE_PATH, .path_len = sizeof(VILLAGE_PATH) / sizeof(int) / 2,
        .total_waves = 15, .boss_kind = NO_BOSS,
    },
    {
        .display_name = "Ravine", .tile_tint_r = 230, .tile_tint_g = 210, .tile_tint_b = 180,
        .width = 12, .height = 6, .tiles = (const int *)RAVINE_TILES,
        .path_xy = RAVINE_PATH, .path_len = sizeof(RAVINE_PATH) / sizeof(int) / 2,
        .total_waves = 15, .boss_kind = NO_BOSS,
    },
    {
        .display_name = "Ice Peak", .tile_tint_r = 200, .tile_tint_g = 222, .tile_tint_b = 255,
        .width = 12, .height = 6, .tiles = (const int *)ICE_PEAK_TILES,
        .path_xy = ICE_PEAK_PATH, .path_len = sizeof(ICE_PEAK_PATH) / sizeof(int) / 2,
        .total_waves = 15, .boss_kind = NO_BOSS,
    },
    {
        .display_name = "Abandoned Mine", .tile_tint_r = 190, .tile_tint_g = 178, .tile_tint_b = 165,
        .width = 12, .height = 6, .tiles = (const int *)ABANDONED_MINE_TILES,
        .path_xy = ABANDONED_MINE_PATH, .path_len = sizeof(ABANDONED_MINE_PATH) / sizeof(int) / 2,
        .total_waves = 15, .boss_kind = NO_BOSS,
    },
    {
        .display_name = "Spider Cavern", .tile_tint_r = 178, .tile_tint_g = 160, .tile_tint_b = 205,
        .width = 12, .height = 6, .tiles = (const int *)SPIDER_CAVERN_TILES,
        .path_xy = SPIDER_CAVERN_PATH, .path_len = sizeof(SPIDER_CAVERN_PATH) / sizeof(int) / 2,
        .total_waves = 15, .boss_kind = NO_BOSS,
    },
    {
        .display_name = "Deserted Beach", .tile_tint_r = 255, .tile_tint_g = 236, .tile_tint_b = 195,
        .width = 12, .height = 6, .tiles = (const int *)DESERTED_BEACH_TILES,
        .path_xy = DESERTED_BEACH_PATH, .path_len = sizeof(DESERTED_BEACH_PATH) / sizeof(int) / 2,
        .total_waves = 15, .boss_kind = NO_BOSS,
    },
    {
        .display_name = "Mountain Ascent", .tile_tint_r = 212, .tile_tint_g = 218, .tile_tint_b = 228,
        .width = 12, .height = 6, .tiles = (const int *)MOUNTAIN_ASCENT_TILES,
        .path_xy = MOUNTAIN_ASCENT_PATH, .path_len = sizeof(MOUNTAIN_ASCENT_PATH) / sizeof(int) / 2,
        .total_waves = 15, .boss_kind = NO_BOSS,
    },
    {
        /* Final map - gets the one-time Herobrine boss wave, see
         * header comment: this pairing is a design choice, not
         * extracted. */
        .display_name = "Nether Stronghold", .tile_tint_r = 255, .tile_tint_g = 150, .tile_tint_b = 140,
        .width = 12, .height = 6, .tiles = (const int *)NETHER_STRONGHOLD_TILES,
        .path_xy = NETHER_STRONGHOLD_PATH, .path_len = sizeof(NETHER_STRONGHOLD_PATH) / sizeof(int) / 2,
        .total_waves = 15, .boss_kind = ENEMY_HEROBRINE, .boss_wave = 15,
    },
};

#undef NO_BOSS

void map_load_into_grid(int map_index, Grid *out_grid) {
    if (map_index < 0 || map_index >= MAP_COUNT) map_index = 0;
    const MapDef *m = &MAP_DEFS[map_index];
    grid_load(out_grid, m->tiles, m->width, m->height, m->path_xy, m->path_len);
}
