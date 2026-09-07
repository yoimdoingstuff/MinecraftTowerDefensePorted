#include "enemy.h"
#include <string.h>
#include <math.h>

const EnemyDef ENEMY_DEFS[ENEMY_COUNT] = {
    [ENEMY_ZOMBIE]       = {"Zombie",       "images/enemy_zombie.png",       0.6f, 0,   10,  0},
    [ENEMY_SKELETON]     = {"Skeleton",     "images/enemy_skeleton.png",     0.6f, 0,   10,  0},
    [ENEMY_SPIDER]       = {"Spider",       "images/enemy_spider.png",       1.0f, 0,   10,  0},
    [ENEMY_CREEPER]      = {"Creeper",      "images/enemy_creeper.png",      0.5f, 0,   10,  0},
    [ENEMY_CAVE_SPIDER]  = {"Cave Spider",  "images/enemy_cave_spider.png",  1.1f, 0,   10,  0},
    [ENEMY_SILVERFISH]   = {"Silverfish",   "images/enemy_silverfish.png",   0.8f, 0,   10,  0},
    [ENEMY_ZOMBIE_PIG]   = {"Zombie Pig",   "images/enemy_zombie_pig.png",   0.7f, 0,   10,  0},
    [ENEMY_BLAZE]        = {"Blaze",        "images/enemy_blaze.png",        0.6f, 0,   10,  0},
    [ENEMY_GHAST]        = {"Ghast",        "images/enemy_ghast.png",        0.4f, 0,   10,  0},
    [ENEMY_MAGMA]        = {"Magma Cube",   "images/enemy_magma.png",        0.3f, 0,   10,  0},
    [ENEMY_SLIME]        = {"Slime",        "images/enemy_slime.png",        0.3f, 0,   10,  0},
    [ENEMY_SPIDER_JOCKEY]= {"Spider Jockey","images/enemy_spider_jockey.png",1.0f, 0,   10,  0},
    [ENEMY_ENDERMAN]     = {"Enderman",     "images/enemy_enderman.png",     4.0f, 0,   10,  0},
    [ENEMY_HEROBRINE]    = {"Herobrine",    "images/enemy_herobrine.png",    0.6f, 750, 500, 1},
};

void enemy_set_init(EnemySet *es) {
    memset(es, 0, sizeof(*es));
    es->next_id = 1;
}

int enemy_set_spawn(EnemySet *es, const Grid *grid, EnemyKind kind, int wave_number) {
    if (grid->path_len < 1) return -1;

    /* Reuse a dead slot if one exists so a long session doesn't run
     * out of room just because many enemies have already died. */
    Enemy *e = 0;
    for (int i = 0; i < es->count; i++) {
        if (!es->list[i].alive) { e = &es->list[i]; break; }
    }
    if (!e) {
        if (es->count >= ENEMY_MAX_ACTIVE) return -1;
        e = &es->list[es->count++];
    }
    memset(e, 0, sizeof(*e));
    e->kind = kind;
    e->waypoint_index = 1 < grid->path_len ? 1 : 0;
    {
        int px, py;
        grid_tile_to_pixel(grid->path_x[0], grid->path_y[0], &px, &py);
        e->px = (float)px + TILE_SIZE * 0.5f;
        e->py = (float)py + TILE_SIZE * 0.5f;
    }
    const EnemyDef *def = &ENEMY_DEFS[kind];
    e->max_hp = def->fixed_hp ? def->fixed_hp : wave_number;
    if (e->max_hp < 1) e->max_hp = 1;
    e->hp = e->max_hp;
    e->alive = 1;
    e->id = es->next_id++;
    return e->id;
}

Enemy *enemy_set_find(EnemySet *es, int id) {
    for (int i = 0; i < es->count; i++) {
        if (es->list[i].alive && es->list[i].id == id) return &es->list[i];
    }
    return 0;
}

void enemy_set_update(EnemySet *es, const Grid *grid, float dt, int *out_leaked_count) {
    int leaked = 0;
    for (int i = 0; i < es->count; i++) {
        Enemy *e = &es->list[i];
        if (!e->alive) continue;

        if (e->waypoint_index >= grid->path_len) {
            /* already at/past the last waypoint - shouldn't normally
             * still be alive here, but guard against it anyway */
            e->alive = 0;
            e->reached_exit = 1;
            leaked++;
            continue;
        }

        int wx, wy;
        grid_tile_to_pixel(grid->path_x[e->waypoint_index], grid->path_y[e->waypoint_index], &wx, &wy);
        float target_x = (float)wx + TILE_SIZE * 0.5f;
        float target_y = (float)wy + TILE_SIZE * 0.5f;

        float dx = target_x - e->px;
        float dy = target_y - e->py;
        float dist = sqrtf(dx * dx + dy * dy);
        float step = ENEMY_DEFS[e->kind].speed * TILE_SIZE * dt;

        if (dist <= step) {
            e->px = target_x;
            e->py = target_y;
            e->waypoint_index++;
            if (e->waypoint_index >= grid->path_len) {
                e->alive = 0;
                e->reached_exit = 1;
                leaked++;
            }
        } else {
            e->px += dx / dist * step;
            e->py += dy / dist * step;
        }
    }
    if (out_leaked_count) *out_leaked_count = leaked;
}
