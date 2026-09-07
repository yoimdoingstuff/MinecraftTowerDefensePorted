#include "render.h"
#include "map_data.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* The built-in font is uppercase-only; HUD strings go through this
 * before drawing rather than shipping a second (lowercase) glyph set. */
static void draw_text_upper(int x, int y, const char *s, uint8_t r, uint8_t g, uint8_t b) {
    char buf[64];
    size_t n = strlen(s);
    if (n > sizeof(buf) - 1) n = sizeof(buf) - 1;
    for (size_t i = 0; i < n; i++) buf[i] = (char)toupper((unsigned char)s[i]);
    buf[n] = '\0';
    platform_draw_text(x, y, buf, r, g, b);
}

static PlatformTexture *tex_for_tile(const Assets *a, TileType t) {
    return (t == TILE_PATH) ? a->tile_path : a->tile_buildable;
}

void render_game(const Game *g, const Assets *a) {
    platform_clear(18, 18, 24);

    const MapDef *map = &MAP_DEFS[g->map_index];
    for (int y = 0; y < g->grid.height; y++) {
        for (int x = 0; x < g->grid.width; x++) {
            TileType t = g->grid.tiles[y][x];
            if (t == TILE_BLOCKED) continue;
            int px, py;
            grid_tile_to_pixel(x, y, &px, &py);
            platform_draw_texture(tex_for_tile(a, t), px, py);
        }
    }
    /* Per-map mood tint: a translucent wash over the ground only (not
     * towers/enemies/UI), so maps read as visually distinct without
     * destroying the buildable-vs-path color distinction the way
     * multiplying each tile's own texture by the tint did (tried
     * that first - it can only darken a colored texture, never shift
     * its hue, and it flattened buildable/path into the same cast). */
    if (map->tile_tint_r != 255 || map->tile_tint_g != 255 || map->tile_tint_b != 255) {
        platform_draw_rect(0, 0, g->grid.width * TILE_SIZE, g->grid.height * TILE_SIZE,
                            map->tile_tint_r, map->tile_tint_g, map->tile_tint_b, 60);
    }

    if (g->status == GAME_PLAYING) {
        int px, py;
        grid_tile_to_pixel(g->cursor_x, g->cursor_y, &px, &py);
        int can_place = grid_is_buildable(&g->grid, g->cursor_x, g->cursor_y) &&
                         !tower_set_occupied(&g->towers, g->cursor_x, g->cursor_y) &&
                         g->currency >= g->next_cost[g->selected_tower];
        if (can_place) {
            platform_draw_texture(a->cursor, px, py);
        } else {
            platform_draw_texture_tinted(a->cursor, px, py, 255, 90, 90);
        }
    }

    for (int i = 0; i < g->towers.count; i++) {
        const Tower *t = &g->towers.list[i];
        if (!t->alive) continue;
        int px, py;
        grid_tile_to_pixel(t->tile_x, t->tile_y, &px, &py);
        platform_draw_texture(a->tower_tex[t->kind], px, py);
        if (t->fire_flash > 0.0f) {
            platform_draw_rect(px, py, TILE_SIZE, TILE_SIZE, 255, 255, 255, 90);
        }
        int tier = tower_tier_for_kills(g->tower_lifetime_kills[t->kind]);
        if (tier > 5) tier = 5; /* provably true already, just satisfies -Wformat-truncation */
        if (tier > 1) {
            char tierbuf[8];
            snprintf(tierbuf, sizeof tierbuf, "%d", tier);
            platform_draw_rect(px + TILE_SIZE - 11, py + TILE_SIZE - 11, 11, 11, 20, 20, 20, 210);
            draw_text_upper(px + TILE_SIZE - 9, py + TILE_SIZE - 9, tierbuf, 255, 220, 90);
        }
    }

    for (int i = 0; i < GAME_MAX_SHOT_FX; i++) {
        if (!g->shot_fx[i].active) continue;
        float p = 1.0f - (g->shot_fx[i].ttl / 0.12f);
        if (p < 0.0f) p = 0.0f;
        if (p > 1.0f) p = 1.0f;
        float x = g->shot_fx[i].x0 + (g->shot_fx[i].x1 - g->shot_fx[i].x0) * p;
        float y = g->shot_fx[i].y0 + (g->shot_fx[i].y1 - g->shot_fx[i].y0) * p;
        platform_draw_texture(a->projectile, (int)x - 4, (int)y - 4);
    }

    for (int i = 0; i < g->enemies.count; i++) {
        const Enemy *e = &g->enemies.list[i];
        if (!e->alive) continue;
        PlatformTexture *tex = a->enemy_tex[e->kind];
        int w = 32, h = 32;
        platform_texture_size(tex, &w, &h);
        int ex = (int)e->px - w / 2;
        int ey = (int)e->py - h / 2;
        platform_draw_texture(tex, ex, ey);

        float frac = e->max_hp > 0 ? (float)e->hp / (float)e->max_hp : 0.0f;
        if (frac < 0.0f) frac = 0.0f;
        platform_draw_rect(ex, ey - 5, w, 3, 50, 10, 10, 255);
        platform_draw_rect(ex, ey - 5, (int)(w * frac), 3, 60, 210, 70, 255);
    }

    int grid_h_px = g->grid.height * TILE_SIZE;
    int grid_w_px = g->grid.width * TILE_SIZE;
    platform_draw_rect(0, grid_h_px, grid_w_px, 32, 12, 12, 16, 255);

    /* Column layout tuned against the real 480px width at 2x font
     * scale (12px/char) so the longest possible string in each column
     * (worst case "ENDERPEARL 2500") can't run into the next column -
     * verified against an actual render, see dev notes. */
    char buf[64];
    snprintf(buf, sizeof buf, "GOLD %d", g->currency);
    draw_text_upper(4, grid_h_px + 4, buf, 255, 215, 80);

    snprintf(buf, sizeof buf, "LIVES %d", g->lives);
    draw_text_upper(4, grid_h_px + 18, buf, 220, 90, 90);

    int total = MAP_DEFS[g->map_index].total_waves;
    if (total > 0) snprintf(buf, sizeof buf, "WAVE %d/%d", g->wave.wave_number, total);
    else snprintf(buf, sizeof buf, "WAVE %d", g->wave.wave_number);
    draw_text_upper(170, grid_h_px + 4, buf, 210, 210, 220);

    if (g->wave.state == WAVE_IDLE) {
        draw_text_upper(170, grid_h_px + 18, "READY", 150, 220, 150);
    }

    const TowerDef *sel = &TOWER_DEFS[g->selected_tower];
    int sel_tier = tower_tier_for_kills(g->tower_lifetime_kills[g->selected_tower]);
    /* Shorter name cap when the "Tn" suffix is present too, so the
     * worst case (e.g. "ENDERPEARL" + "2500" + "T5") still fits the
     * ~150px budget between here and the screen edge. */
    char short_name[8];
    int name_cap = (sel_tier > 1) ? 4 : 7;
    strncpy(short_name, sel->name, name_cap);
    short_name[name_cap] = '\0';
    if (sel_tier > 1) snprintf(buf, sizeof buf, "%s %d T%d", short_name, g->next_cost[g->selected_tower], sel_tier);
    else snprintf(buf, sizeof buf, "%s %d", short_name, g->next_cost[g->selected_tower]);
    draw_text_upper(330, grid_h_px + 4, buf, 200, 200, 255);
    draw_text_upper(330, grid_h_px + 18, "R:WAVE", 140, 140, 160);

    if (g->status == GAME_PAUSED) {
        platform_draw_rect(0, 0, grid_w_px, grid_h_px, 0, 0, 0, 140);
        draw_text_upper(grid_w_px / 2 - 30, grid_h_px / 2, "PAUSED", 255, 255, 255);
    } else if (g->status == GAME_WON) {
        platform_draw_rect(0, 0, grid_w_px, grid_h_px, 0, 40, 0, 160);
        draw_text_upper(grid_w_px / 2 - 40, grid_h_px / 2, "YOU WIN!", 220, 255, 220);
    } else if (g->status == GAME_LOST) {
        platform_draw_rect(0, 0, grid_w_px, grid_h_px, 40, 0, 0, 160);
        draw_text_upper(grid_w_px / 2 - 45, grid_h_px / 2, "GAME OVER", 255, 220, 220);
    }

    platform_present();
}
