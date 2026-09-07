/* Tower ("dispenser") types.
 *
 * Base stats below are taken from the decompiled AS2 (see
 * docs/original-game-reference.md) - cost, power, range and firerate
 * are the real values from each dispenser's onLoad(). Things NOT
 * carried over 1:1, called out explicitly:
 *
 *  - Escalating per-purchase pricing (buying another of the same
 *    tower raises that type's price for next time) is implemented in
 *    game.c, not here - TOWER_DEFS.base_cost is the price of the
 *    FIRST one only.
 *  - TNT Dispenser's power wasn't in a simple `this.power = N` line in
 *    the source (it likely computes an explosion/AoE value elsewhere);
 *    the value here is an estimate marked below, not an extracted
 *    number. Treat it as a placeholder to tune.
 *  - Tier system: the reference doc confirms a persistent, lifetime-
 *    kills-based tier 1-5 system exists, with arrow's exact
 *    thresholds (100/250/500/1000 kills) extracted. Applying arrow's
 *    thresholds to every tower type (below) is an assumption, not a
 *    per-type confirmed fact. The stat bonus per tier (+15%
 *    power/+10% range per tier above 1, in tower_tier_power_mult /
 *    tower_tier_range_mult) is entirely a design choice - the
 *    original's exact tier bonus formula was never traced, only that
 *    a visual upgrade happens at some threshold. Kill counts are
 *    in-memory only for now (reset each run) - persisting them
 *    across sessions needs a save file, not added yet.
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

#define TOWER_MAX_TIER 5
/* Lifetime kills needed to reach tiers 2..5 (index 0 = threshold for
 * tier 2, etc). Extracted for arrow specifically; applied to all
 * towers as an assumption - see header comment. */
extern const int TOWER_TIER_KILL_THRESHOLDS[TOWER_MAX_TIER - 1];

int tower_tier_for_kills(int lifetime_kills);
float tower_tier_power_mult(int tier);
float tower_tier_range_mult(int tier);

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
