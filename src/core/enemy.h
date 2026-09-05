/* Enemy types.
 *
 * Speeds are the real extracted values. HP follows the original's rule:
 * health = current wave number, for every enemy EXCEPT the ones marked
 * fixed_hp below (allies with flat low HP, and Herobrine as a fixed
 * 750HP boss that never appears in this starter roster).
 *
 * This starter build includes the early-wave roster only (see
 * docs/original-game-reference.md for the full ~16-type list); the
 * rest are documented there and are a data-table addition away, not
 * new engine work.
 */
#ifndef MTD_ENEMY_H
#define MTD_ENEMY_H

#include "grid.h"

typedef enum {
    ENEMY_ZOMBIE = 0,
    ENEMY_SKELETON,
    ENEMY_SPIDER,
    ENEMY_CREEPER,
    ENEMY_CAVE_SPIDER,
    ENEMY_SILVERFISH,
    ENEMY_COUNT
} EnemyKind;

typedef struct {
    const char *name;
    const char *sprite_asset;
    float speed;         /* tiles per second */
    int fixed_hp;         /* 0 = use current wave number; nonzero = always this many HP */
    int reward;           /* currency on death - only zombie's 10 is a confirmed
                              extracted value; others reuse it as a baseline until
                              individually confirmed (see reference doc) */
} EnemyDef;

extern const EnemyDef ENEMY_DEFS[ENEMY_COUNT];

#define ENEMY_MAX_ACTIVE 96

typedef struct {
    EnemyKind kind;
    float px, py;        /* pixel position */
    int waypoint_index;  /* next path waypoint to head toward */
    int hp, max_hp;
    int alive;
    int reached_exit;    /* 1 if it got through instead of dying */
    int id;               /* stable id for tower targeting, not reused while alive */
} Enemy;

typedef struct {
    Enemy list[ENEMY_MAX_ACTIVE];
    int count;
    int next_id;
} EnemySet;

void enemy_set_init(EnemySet *es);
int enemy_set_spawn(EnemySet *es, const Grid *grid, EnemyKind kind, int wave_number);
void enemy_set_update(EnemySet *es, const Grid *grid, float dt,
                       int *out_leaked_count);
Enemy *enemy_set_find(EnemySet *es, int id);

#endif /* MTD_ENEMY_H */
