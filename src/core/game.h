#ifndef MTD_GAME_H
#define MTD_GAME_H

#include "grid.h"
#include "tower.h"
#include "enemy.h"
#include "wave.h"
#include "platform.h"

typedef enum { GAME_PLAYING, GAME_WON, GAME_LOST, GAME_PAUSED } GameStatus;

typedef struct {
    Grid grid;
    TowerSet towers;
    EnemySet enemies;
    WaveManager wave;

    int map_index;
    int currency;
    int lives;         /* starting currency/lives are NOT extracted values -
                          see game.c game_init() comment for the defaults used */
    GameStatus status;

    int cursor_x, cursor_y;         /* tile coords */
    TowerKind selected_tower;

    /* Escalating per-purchase price, one counter per tower type -
     * starts at TOWER_DEFS[k].base_cost, +50 each purchase (matches
     * the extracted arrow_dispenser behavior; applied to all types
     * for consistency since only arrow's exact delta was confirmed -
     * see docs/original-game-reference.md). */
    int next_cost[TOWER_COUNT];

    /* Lifetime kills per tower type, driving the tier 1-5 system (see
     * tower.h). In-memory only - resets each run, not saved yet. */
    int tower_lifetime_kills[TOWER_COUNT];

    float speed_multiplier;   /* 1.0 normal, 2.0 after speed-toggle */
    int shots_fired_this_frame; /* reset each game_update() call; backends
                                    use this to trigger the fire sound once
                                    per frame regardless of how many towers
                                    fired, without game.c needing to know
                                    about Assets/sounds at all */

    /* Cosmetic-only shot effects (tower->target tracer), no gameplay
     * effect - damage is resolved instantly when a tower fires.
     * Cursor auto-repeat is the input backend's job, not game.c's -
     * PlatformInput.move_x/y already arrives debounced. */
    #define GAME_MAX_SHOT_FX 32
    struct {
        float x0, y0, x1, y1, ttl;
        int active;
    } shot_fx[GAME_MAX_SHOT_FX];
} Game;

void game_init(Game *g, int map_index);
void game_handle_input(Game *g, const PlatformInput *input, float dt);
void game_update(Game *g, float dt);

#endif /* MTD_GAME_H */
