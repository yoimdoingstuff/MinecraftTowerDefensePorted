/* Wave progression.
 *
 * The original computes enemies-per-wave with a separate formula per
 * map (13 story maps each had their own curve - see
 * docs/original-game-reference.md). We haven't extracted every map's
 * exact formula yet, so wave_enemy_count() below is a reasonable
 * approximation (grows with wave number, capped) rather than an
 * extracted constant - flagged here so it doesn't get mistaken for a
 * confirmed number later. What IS extracted and used exactly: HP per
 * enemy equals the current wave number (handled in enemy.c), and
 * survival-style maps run indefinitely (total_waves acts as infinite).
 */
#ifndef MTD_WAVE_H
#define MTD_WAVE_H

#include "enemy.h"

typedef enum {
    WAVE_IDLE,       /* waiting for the player to start the next wave */
    WAVE_SPAWNING,   /* still releasing this wave's enemies */
    WAVE_IN_PROGRESS /* all spawned, waiting for them to die/leak */
} WaveState;

typedef struct {
    int wave_number;         /* 1-based */
    int total_waves;         /* 0 = endless/survival */
    WaveState state;
    int to_spawn;             /* enemies left to release this wave */
    int spawned_alive;        /* enemies released but not yet dead/leaked */
    float spawn_timer;
    float spawn_interval;     /* seconds between spawns within a wave */
} WaveManager;

void wave_manager_init(WaveManager *wm, int total_waves);
/* Call once the player confirms "start next wave" (or auto-start, if
 * that's how the caller wants to drive it) while state == WAVE_IDLE. */
void wave_manager_begin_wave(WaveManager *wm);

/* Roughly how many enemies wave N should contain. See header comment -
 * approximation, not an extracted per-map formula. */
int wave_enemy_count(int wave_number);

/* Simple deterministic mix of the 6-enemy starter roster so early
 * waves lean on zombies/skeletons and later ones bring in the faster
 * types. Not extracted - a placeholder rule to tune later. */
EnemyKind wave_pick_enemy_kind(int wave_number, int spawn_index_in_wave);

/* Advances spawning; call every frame. Releases enemies one at a time
 * every spawn_interval seconds until the wave's count is used up. */
void wave_manager_update(WaveManager *wm, EnemySet *es, const Grid *grid, float dt);

/* Call once per frame with how many of THIS wave's enemies died or
 * leaked since the last call (game.c knows this - it's the one
 * checking hp<=0 and reached_exit on the enemy set). Since a new wave
 * never starts spawning until the previous one's count reaches zero,
 * every currently-alive enemy always belongs to the current wave, so
 * a simple resolved-count is enough to know when the wave is over. */
void wave_manager_notify_resolved(WaveManager *wm, int resolved_count);

int wave_manager_is_wave_over(const WaveManager *wm);

#endif /* MTD_WAVE_H */
