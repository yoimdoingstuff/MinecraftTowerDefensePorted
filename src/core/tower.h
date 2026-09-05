/* Tower ("dispenser") types.
 *
 * Base stats below are taken from the decompiled AS2 (see
 * docs/original-game-reference.md) - cost, power, range and firerate
 * are the real values from each dispenser's onLoad(). Two things are
 * NOT carried over 1:1 and are called out explicitly:
 *
 *  - Escalating per-purchase pricing (buying another of the same
 *    tower raises that type's price for next time) is implemented in
 *    game.c, not here - TOWER_DEFS.base_cost is the price of the
 *    FIRST one only.
 *  - TNT Dispenser's power wasn't in a simple `this.power = N` line in
 *    the source (it likely computes an explosion/AoE value elsewhere);
 *    the value here is an estimate marked below, not an extracted
 *    number. Treat it as a placeholder to tune.
 *  - The permanent lifetime-kills tier system (tiers 1-5 per tower,
 *    unlocked by cumulative kills across sessions) is not implemented
 *    yet - towers always play at their tier-1 stats for now.
 */
#ifndef MTD_TOWER_H
#define MTD_TOWER_H

#include "grid.h"

typedef enum {
    TOWER_EGG = 0,
    TOWER_SNOW,
    TOWER_ARROW,
    TOWER_FIREBALL,
    TOWER_SLIME,
    TOWER_ENDERPEARL,
    TOWER_GOLDEN,
    TOWER_TNT,
    TOWER_POISON,
    TOWER_COUNT
} TowerKind;

typedef struct {
    const char *name;
    const char *icon_asset;   /* relative path under assets/images */
    int   base_cost;          /* price of the first one placed */
    float power;              /* damage per hit */
    float range;              /* in tiles */
    float firerate;           /* shots per second */
    int   is_estimated_power; /* 1 = not a directly extracted number, see header comment */
} TowerDef;

extern const TowerDef TOWER_DEFS[TOWER_COUNT];

#define TOWER_MAX_ACTIVE 64

typedef struct {
    TowerKind kind;
    int tile_x, tile_y;
    float cooldown;       /* seconds until it can fire again */
    int target_enemy_id;  /* -1 if none */
    int cost_paid;         /* what this specific tower cost, for sell refunds */
    float fire_flash;      /* cosmetic: >0 briefly after firing, counts down */
    int alive;
} Tower;

typedef struct {
    Tower list[TOWER_MAX_ACTIVE];
    int count;
} TowerSet;

void tower_set_init(TowerSet *ts);
/* Returns the new tower's index, or -1 if the tile isn't buildable/
 * already occupied or TOWER_MAX_ACTIVE was reached. Does not touch
 * currency - the caller (game.c) checks/deducts cost first. */
int tower_set_place(TowerSet *ts, const Grid *grid, TowerKind kind, int tx, int ty, int cost_paid);
void tower_set_remove(TowerSet *ts, int index);
int tower_set_occupied(const TowerSet *ts, int tx, int ty);

#endif /* MTD_TOWER_H */
