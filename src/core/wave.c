#include "wave.h"
#include <string.h>

void wave_manager_init(WaveManager *wm, int total_waves) {
    memset(wm, 0, sizeof(*wm));
    wm->wave_number = 0;
    wm->total_waves = total_waves;
    wm->state = WAVE_IDLE;
    wm->spawn_interval = 0.8f;
}

int wave_enemy_count(int wave_number) {
    int count = 3 + wave_number * 2; /* approximation - see header */
    if (count > 30) count = 30;
    return count;
}

EnemyKind wave_pick_enemy_kind(int wave_number, int spawn_index_in_wave) {
    /* Very simple unlock-by-wave rule: early waves are all zombies,
     * later waves mix in the rest of the starter roster. Placeholder,
     * not extracted - easy to replace with real per-map spawn tables
     * once those are pulled from the SWF. */
    if (wave_number < 2) return ENEMY_ZOMBIE;
    if (wave_number < 4) return (spawn_index_in_wave % 2 == 0) ? ENEMY_ZOMBIE : ENEMY_SKELETON;
    int roster[ENEMY_COUNT] = {ENEMY_ZOMBIE, ENEMY_SKELETON, ENEMY_SPIDER,
                                ENEMY_CREEPER, ENEMY_CAVE_SPIDER, ENEMY_SILVERFISH};
    int unlocked = 3 + wave_number / 2;
    if (unlocked > ENEMY_COUNT) unlocked = ENEMY_COUNT;
    return (EnemyKind)roster[spawn_index_in_wave % unlocked];
}

void wave_manager_begin_wave(WaveManager *wm) {
    if (wm->state != WAVE_IDLE) return;
    wm->wave_number++;
    wm->to_spawn = wave_enemy_count(wm->wave_number);
    wm->spawned_alive = 0;
    wm->spawn_timer = 0.0f;
    wm->state = WAVE_SPAWNING;
}

void wave_manager_update(WaveManager *wm, EnemySet *es, const Grid *grid, float dt) {
    if (wm->state != WAVE_SPAWNING) return;

    wm->spawn_timer += dt;
    while (wm->spawn_timer >= wm->spawn_interval && wm->to_spawn > 0) {
        wm->spawn_timer -= wm->spawn_interval;
        int spawn_index = wave_enemy_count(wm->wave_number) - wm->to_spawn;
        EnemyKind kind = wave_pick_enemy_kind(wm->wave_number, spawn_index);
        if (enemy_set_spawn(es, grid, kind, wm->wave_number) >= 0) {
            wm->spawned_alive++;
        }
        wm->to_spawn--;
    }
    if (wm->to_spawn <= 0) {
        wm->state = WAVE_IN_PROGRESS;
    }
}

void wave_manager_notify_resolved(WaveManager *wm, int resolved_count) {
    if (resolved_count <= 0) return;
    wm->spawned_alive -= resolved_count;
    if (wm->spawned_alive < 0) wm->spawned_alive = 0;
    if (wm->state == WAVE_IN_PROGRESS && wm->spawned_alive == 0) {
        wm->state = WAVE_IDLE;
    }
}

int wave_manager_is_wave_over(const WaveManager *wm) {
    return wm->state == WAVE_IDLE && wm->wave_number > 0;
}
