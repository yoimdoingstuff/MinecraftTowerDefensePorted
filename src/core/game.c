#include "game.h"
#include "map_data.h"
#include <string.h>

static void add_shot_fx(Game *g, float x0, float y0, float x1, float y1) {
    for (int i = 0; i < GAME_MAX_SHOT_FX; i++) {
        if (!g->shot_fx[i].active) {
            g->shot_fx[i].active = 1;
            g->shot_fx[i].x0 = x0; g->shot_fx[i].y0 = y0;
            g->shot_fx[i].x1 = x1; g->shot_fx[i].y1 = y1;
            g->shot_fx[i].ttl = 0.12f;
            return;
        }
    }
    /* fx pool full - just drop it, purely cosmetic */
}

void game_init(Game *g, int map_index) {
    memset(g, 0, sizeof(*g));
    if (map_index < 0 || map_index >= MAP_COUNT) map_index = 0;
    g->map_index = map_index;
    map_load_into_grid(map_index, &g->grid);
    tower_set_init(&g->towers);
    enemy_set_init(&g->enemies);
    wave_manager_init(&g->wave, MAP_DEFS[map_index].total_waves,
                       MAP_DEFS[map_index].boss_kind, MAP_DEFS[map_index].boss_wave);

    /* Starting currency/lives are NOT extracted values - the original's
     * exact starting numbers weren't pinned down in this pass. 300
     * currency covers a couple of early towers (egg 30 + snow 100 +
     * arrow 150 = 280); 20 lives is a plain reasonable default. Both
     * are one-line changes here once real numbers are found. */
    g->currency = 300;
    g->lives = 20;
    g->status = GAME_PLAYING;
    g->cursor_x = 0;
    g->cursor_y = 0;
    g->selected_tower = TOWER_ARROW;
    g->speed_multiplier = 1.0f;

    for (int i = 0; i < TOWER_COUNT; i++) g->next_cost[i] = TOWER_DEFS[i].base_cost;
}

void game_handle_input(Game *g, const PlatformInput *input, float dt) {
    (void)dt;

    if (input->just_pause) {
        if (g->status == GAME_PLAYING) g->status = GAME_PAUSED;
        else if (g->status == GAME_PAUSED) g->status = GAME_PLAYING;
    }

    if (g->status != GAME_PLAYING) return; /* paused/won/lost: only pause toggle above works */

    int nx = g->cursor_x + input->move_x;
    int ny = g->cursor_y + input->move_y;
    if (nx >= 0 && nx < g->grid.width) g->cursor_x = nx;
    if (ny >= 0 && ny < g->grid.height) g->cursor_y = ny;

    if (input->just_next_tower) g->selected_tower = (TowerKind)((g->selected_tower + 1) % TOWER_COUNT);
    if (input->just_prev_tower) g->selected_tower = (TowerKind)((g->selected_tower + TOWER_COUNT - 1) % TOWER_COUNT);
    if (input->just_speed_toggle) g->speed_multiplier = (g->speed_multiplier > 1.0f) ? 1.0f : 2.0f;
    if (input->just_next_wave && g->wave.state == WAVE_IDLE) wave_manager_begin_wave(&g->wave);

    if (input->just_confirm) {
        TowerKind k = g->selected_tower;
        int cost = g->next_cost[k];
        if (g->currency >= cost && grid_is_buildable(&g->grid, g->cursor_x, g->cursor_y) &&
            !tower_set_occupied(&g->towers, g->cursor_x, g->cursor_y)) {
            int idx = tower_set_place(&g->towers, &g->grid, k, g->cursor_x, g->cursor_y, cost);
            if (idx >= 0) {
                g->currency -= cost;
                g->next_cost[k] += 50; /* matches the extracted arrow_dispenser escalation */
            }
        }
    }

    if (input->just_cancel) {
        /* Sell the tower under the cursor, 50% refund. The original's
         * exact sell formula wasn't re-confirmed for this pass (see
         * docs/original-game-reference.md) - this uses a standard TD
         * sell-back rate rather than guessing at the precise number. */
        for (int i = 0; i < g->towers.count; i++) {
            Tower *t = &g->towers.list[i];
            if (t->alive && t->tile_x == g->cursor_x && t->tile_y == g->cursor_y) {
                g->currency += t->cost_paid / 2;
                g->next_cost[t->kind] -= 50;
                if (g->next_cost[t->kind] < TOWER_DEFS[t->kind].base_cost)
                    g->next_cost[t->kind] = TOWER_DEFS[t->kind].base_cost;
                tower_set_remove(&g->towers, i);
                break;
            }
        }
    }
}

/* Finds the nearest enemy in front of a tower along whichever cardinal
 * direction(s) touch the lane, honoring range and line-of-sight
 * (a non-path tile in the way shortens that direction's effective
 * range - matches the extracted restrict_range() behavior). Range is
 * adjusted by the tower type's current tier (see tower.h). */
static int find_target_for_tower(const Game *g, const Tower *t) {
    static const int dirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
    float tower_cx = (t->tile_x + 0.5f) * TILE_SIZE;
    float tower_cy = (t->tile_y + 0.5f) * TILE_SIZE;
    const TowerDef *def = &TOWER_DEFS[t->kind];
    int tier = tower_tier_for_kills(g->tower_lifetime_kills[t->kind]);
    float tier_range = def->range * tower_tier_range_mult(tier);
    float range_px = tier_range * TILE_SIZE;

    int best_id = -1;
    float best_along = 1e9f;

    for (int d = 0; d < 4; d++) {
        int dx = dirs[d][0], dy = dirs[d][1];
        if (!grid_path_adjacent(&g->grid, t->tile_x, t->tile_y, dx, dy)) continue;

        float effective_range = range_px;
        int steps = (int)tier_range + 1;
        for (int s = 1; s <= steps; s++) {
            int tx = t->tile_x + dx * s;
            int ty = t->tile_y + dy * s;
            if (grid_tile_at(&g->grid, tx, ty) != TILE_PATH) {
                float blocked_at = (s - 1) * (float)TILE_SIZE + TILE_SIZE * 0.5f;
                if (blocked_at < effective_range) effective_range = blocked_at;
                break;
            }
        }

        for (int i = 0; i < g->enemies.count; i++) {
            const Enemy *e = &g->enemies.list[i];
            if (!e->alive) continue;
            float along, perp;
            if (dx != 0) { along = (e->px - tower_cx) * dx; perp = e->py - tower_cy; }
            else         { along = (e->py - tower_cy) * dy; perp = e->px - tower_cx; }
            if (along < 0 || along > effective_range) continue;
            if (perp < -TILE_SIZE * 0.5f || perp > TILE_SIZE * 0.5f) continue;
            if (along < best_along) { best_along = along; best_id = e->id; }
        }
    }
    return best_id;
}

void game_update(Game *g, float dt) {
    if (g->status != GAME_PLAYING) return;

    float sdt = dt * g->speed_multiplier;
    int resolved_this_frame = 0;
    g->shots_fired_this_frame = 0;

    wave_manager_update(&g->wave, &g->enemies, &g->grid, sdt);

    for (int i = 0; i < g->towers.count; i++) {
        Tower *t = &g->towers.list[i];
        if (!t->alive) continue;
        if (t->fire_flash > 0.0f) t->fire_flash -= sdt;
        if (t->cooldown > 0.0f) { t->cooldown -= sdt; continue; }

        int target_id = find_target_for_tower(g, t);
        if (target_id < 0) continue;
        Enemy *target = enemy_set_find(&g->enemies, target_id);
        if (!target) continue;

        const TowerDef *def = &TOWER_DEFS[t->kind];
        int tier = tower_tier_for_kills(g->tower_lifetime_kills[t->kind]);
        float effective_power = def->power * tower_tier_power_mult(tier);
        int dmg = (int)(effective_power + 0.999f); /* round up: fractional power still does >=1 */
        target->hp -= dmg;
        t->cooldown = 1.0f / def->firerate;
        t->fire_flash = 0.15f;
        g->shots_fired_this_frame++;
        add_shot_fx(g, (t->tile_x + 0.5f) * TILE_SIZE, (t->tile_y + 0.5f) * TILE_SIZE, target->px, target->py);

        if (target->hp <= 0) {
            target->alive = 0;
            g->currency += ENEMY_DEFS[target->kind].reward;
            g->tower_lifetime_kills[t->kind]++;
            resolved_this_frame++;
        }
    }

    int leaked = 0;
    enemy_set_update(&g->enemies, &g->grid, sdt, &leaked);
    if (leaked > 0) {
        g->lives -= leaked;
        resolved_this_frame += leaked;
    }
    wave_manager_notify_resolved(&g->wave, resolved_this_frame);

    for (int i = 0; i < GAME_MAX_SHOT_FX; i++) {
        if (g->shot_fx[i].active) {
            g->shot_fx[i].ttl -= sdt;
            if (g->shot_fx[i].ttl <= 0.0f) g->shot_fx[i].active = 0;
        }
    }

    if (g->lives <= 0) {
        g->lives = 0;
        g->status = GAME_LOST;
        return;
    }

    int total_waves = MAP_DEFS[g->map_index].total_waves;
    if (total_waves > 0 && g->wave.wave_number >= total_waves && g->wave.state == WAVE_IDLE) {
        g->status = GAME_WON;
    }
}
