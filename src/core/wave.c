#include "wave.h"
#include <string.h>

void wave_manager_init(WaveManager *wm, int total_waves, EnemyKind boss_kind, int boss_wave) {
    memset(wm, 0, sizeof(*wm));
    wm->wave_number = 0;
    wm->total_waves = total_waves;
    wm->state = WAVE_IDLE;
    wm->spawn_interval = 0.8f;
    wm->boss_kind = boss_kind;
    wm->boss_wave = boss_wave;
}

int wave_enemy_count(int wave_number) {
    int count = 3 + wave_number * 2; /* approximation - see header */
    if (count > 30) count = 30;
    return count;
}

EnemyKind wave_pick_enemy_kind(int wave_number, int spawn_index_in_wave) {
    /* Difficulty-ordered roster (slow/simple first, fast/dangerous
     * last). Herobrine is deliberately absent - he's boss-only. */
    static const EnemyKind roster[] = {
        ENEMY_ZOMBIE, ENEMY_SKELETON, ENEMY_CREEPER, ENEMY_SPIDER,
        ENEMY_CAVE_SPIDER, ENEMY_SILVERFISH, ENEMY_ZOMBIE_PIG, ENEMY_BLAZE,
        ENEMY_GHAST, ENEMY_MAGMA, ENEMY_SLIME, ENEMY_SPIDER_JOCKEY, ENEMY_ENDERMAN,
    };
    const int roster_len = (int)(sizeof(roster) / sizeof(roster[0]));

    if (wave_number < 2) return ENEMY_ZOMBIE;
    if (wave_number < 4) return (spawn_index_in_wave % 2 == 0) ? ENEMY_ZOMBIE : ENEMY_SKELETON;

    int unlocked = 3 + wave_number / 2; /* one new type roughly every 2 waves */
    if (unlocked > roster_len) unlocked = roster_len;
    return roster[spawn_index_in_wave % unlocked];
}

void wave_manager_begin_wave(WaveManager *wm) {
    if (wm->state != WAVE_IDLE) return;
    wm->wave_number++;
    wm->current_wave_is_boss = (wm->boss_wave > 0 && wm->wave_number == wm->boss_wave);
    wm->to_spawn = wm->current_wave_is_boss ? 1 : wave_enemy_count(wm->wave_number);
    wm->spawned_alive = 0;
    wm->spawn_timer = 0.0f;
    wm->state = WAVE_SPAWNING;
}

void wave_manager_update(WaveManager *wm, EnemySet *es, const Grid *grid, float dt) {
    if (wm->state != WAVE_SPAWNING) return;

    wm->spawn_timer += dt;
    while (wm->spawn_timer >= wm->spawn_interval && wm->to_spawn > 0) {
        wm->spawn_timer -= wm->spawn_interval;
        EnemyKind kind;
        if (wm->current_wave_is_boss) {
            kind = wm->boss_kind;
        } else {
            int spawn_index = wave_enemy_count(wm->wave_number) - wm->to_spawn;
            kind = wave_pick_enemy_kind(wm->wave_number, spawn_index);
        }
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
