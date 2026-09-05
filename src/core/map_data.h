/* Starter map layouts.
 *
 * IMPORTANT - honesty flag: these three layouts (forest, village,
 * ravine) are ORIGINAL designs built from the confirmed mechanic
 * (fixed 40px grid, single pre-authored lane, buildable plots
 * elsewhere) - they are NOT extracted tile-for-tile from the SWF.
 * Extracting the real per-map tile placement is possible (the map
 * sprites' PlaceObject tags have it) but wasn't done in this pass.
 * Everything else about these maps (background art file, name) IS the
 * real thing; only the exact tile geometry is a stand-in. Swapping in
 * real layouts later only touches this file.
 */
#ifndef MTD_MAP_DATA_H
#define MTD_MAP_DATA_H

#include "grid.h"

typedef struct {
    const char *display_name;
    const char *background_asset; /* relative path under assets/images */
    int width, height;
    const int *tiles;      /* width*height, row-major, TileType values */
    const int *path_xy;    /* path_len*2 ints: x0,y0,x1,y1,... */
    int path_len;
    int total_waves;       /* 0 = endless/survival */
} MapDef;

#define MAP_COUNT 3
extern const MapDef MAP_DEFS[MAP_COUNT];

void map_load_into_grid(int map_index, Grid *out_grid);

#endif /* MTD_MAP_DATA_H */
