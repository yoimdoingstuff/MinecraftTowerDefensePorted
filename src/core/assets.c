#include "assets.h"
#include <string.h>

void assets_load(Assets *a) {
    memset(a, 0, sizeof(*a));
    a->tile_buildable = platform_load_texture("images/tile_buildable.png");
    a->tile_path = platform_load_texture("images/tile_path.png");
    a->cursor = platform_load_texture("images/cursor.png");
    a->projectile = platform_load_texture("images/projectile.png");

    for (int i = 0; i < TOWER_COUNT; i++) {
        a->tower_tex[i] = platform_load_texture(TOWER_DEFS[i].icon_asset);
    }
    for (int i = 0; i < ENEMY_COUNT; i++) {
        a->enemy_tex[i] = platform_load_texture(ENEMY_DEFS[i].sprite_asset);
    }

    a->snd_place = platform_load_sound("audio_wav/pop.wav");
    a->snd_select = platform_load_sound("audio_wav/click.wav");
    a->snd_shoot = platform_load_sound("audio_wav/bow.wav");
}

void assets_free(Assets *a) {
    platform_free_texture(a->tile_buildable);
    platform_free_texture(a->tile_path);
    platform_free_texture(a->cursor);
    platform_free_texture(a->projectile);
    for (int i = 0; i < TOWER_COUNT; i++) platform_free_texture(a->tower_tex[i]);
    for (int i = 0; i < ENEMY_COUNT; i++) platform_free_texture(a->enemy_tex[i]);
    platform_free_sound(a->snd_place);
    platform_free_sound(a->snd_select);
    platform_free_sound(a->snd_shoot);
}
