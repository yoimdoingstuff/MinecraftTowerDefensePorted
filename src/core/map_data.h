/* Starter map layouts.
 *
 * IMPORTANT - honesty flag: these layouts are ORIGINAL designs built
 * from the confirmed mechanic (fixed 40px grid, single pre-authored
 * lane, buildable plots elsewhere) - they are NOT extracted tile-for-
 * tile from the SWF. Extracting the real per-map tile placement is
 * possible (the map sprites' PlaceObject tags have it) but wasn't
 * done in this pass. Swapping in real layouts later only touches this
 * file. Each layout is generated from its waypoint path (not hand-
 * drawn tile by tile), so the grid and path can't disagree with each
 * other the way a hand-transcribed version did once before - see
 * tools/ if adding more.
 *
 * Per-map tint colors are a placeholder for real per-map background
 * art (the original has a distinct painted background per map, e.g.
 * the village screenshot in docs/original-game-reference.md) - a
 * quick way to make maps visually distinct from each other now rather
 * than leaving them all looking identical until real backgrounds are
 * matched from the extracted images.
 *
 * Boss waves: Herobrine (see enemy.h - fixed 750HP, not part of the
 * normal wave pool) is placed as a one-time final-wave encounter on
 * Nether Stronghold. Which map gets the boss and on which wave is a
 * design choice made here, not an extracted fact - the original's
 * exact per-map boss placement (if any) wasn't traced.
 */
#ifndef MTD_MAP_DATA_H
#define MTD_MAP_DATA_H

#include "grid.h"
#include "enemy.h"
#include <stdint.h>

typedef struct {
    const char *display_name;
    uint8_t tile_tint_r, tile_tint_g, tile_tint_b; /* multiplies the base tile colors, see render.c */
    int width, height;
    const int *tiles;      /* width*height, row-major, TileType values */
    const int *path_xy;    /* path_len*2 ints: x0,y0,x1,y1,... */
    int path_len;
    int total_waves;       /* 0 = endless/survival */
    EnemyKind boss_kind;    /* enemy spawned as a one-time boss (see wave.h) */
    int boss_wave;          /* wave number the boss appears on; 0 = no boss */
} MapDef;

#define MAP_COUNT 9
extern const MapDef MAP_DEFS[MAP_COUNT];

void map_load_into_grid(int map_index, Grid *out_grid);

#endif /* MTD_MAP_DATA_H */
