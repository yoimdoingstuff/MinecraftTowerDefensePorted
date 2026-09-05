#include "tower.h"
#include <string.h>

const TowerDef TOWER_DEFS[TOWER_COUNT] = {
    [TOWER_EGG]        = {"Egg Dispenser",        "images/tower_egg.png",        30,   1.0f, 1.0f, 0.5f, 0},
    [TOWER_SNOW]       = {"Snow Dispenser",       "images/tower_snow.png",       100,  1.0f, 0.5f, 0.5f, 0},
    [TOWER_ARROW]      = {"Arrow Dispenser",      "images/tower_arrow.png",      150,  1.0f, 2.5f, 0.5f, 0},
    [TOWER_FIREBALL]   = {"Fireball Dispenser",   "images/tower_fireball.png",   200,  1.0f, 1.0f, 0.5f, 0},
    [TOWER_SLIME]      = {"Slime Dispenser",      "images/tower_slime.png",      250,  1.0f, 2.0f, 0.5f, 0},
    [TOWER_ENDERPEARL] = {"Enderpearl Dispenser", "images/tower_enderpearl.png", 300,  0.5f, 1.0f, 0.5f, 0},
    [TOWER_GOLDEN]     = {"Golden Dispenser",     "images/tower_golden.png",     2500, 5.0f, 5.0f, 2.0f, 0},
    [TOWER_TNT]        = {"TNT Dispenser",        "images/tower_tnt.png",        2500, 3.0f, 2.0f, 0.5f, 1},
    [TOWER_POISON]     = {"Poison Dispenser",     "images/tower_poison.png",     2500, 8.0f, 3.0f, 1.0f, 0},
};

void tower_set_init(TowerSet *ts) {
    memset(ts, 0, sizeof(*ts));
    for (int i = 0; i < TOWER_MAX_ACTIVE; i++) ts->list[i].target_enemy_id = -1;
}

int tower_set_occupied(const TowerSet *ts, int tx, int ty) {
    for (int i = 0; i < ts->count; i++) {
        if (ts->list[i].alive && ts->list[i].tile_x == tx && ts->list[i].tile_y == ty) return 1;
    }
    return 0;
}

int tower_set_place(TowerSet *ts, const Grid *grid, TowerKind kind, int tx, int ty, int cost_paid) {
    if (!grid_is_buildable(grid, tx, ty)) return -1;
    if (tower_set_occupied(ts, tx, ty)) return -1;
    if (ts->count >= TOWER_MAX_ACTIVE) return -1;

    int idx = ts->count++;
    Tower *t = &ts->list[idx];
    t->kind = kind;
    t->tile_x = tx;
    t->tile_y = ty;
    t->cooldown = 0.0f;
    t->target_enemy_id = -1;
    t->cost_paid = cost_paid;
    t->fire_flash = 0.0f;
    t->alive = 1;
    return idx;
}

void tower_set_remove(TowerSet *ts, int index) {
    if (index < 0 || index >= ts->count) return;
    ts->list[index].alive = 0;
}
