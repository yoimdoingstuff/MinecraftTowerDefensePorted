#include "../../core/game.h"
#include "../../core/assets.h"
#include "../../core/render.h"
#include "../../core/map_data.h"

void psp_set_asset_base_from_argv0(const char *argv0); /* platform_psp.c */

int main(int argc, char **argv) {
    if (argc > 0 && argv[0]) psp_set_asset_base_from_argv0(argv[0]);

    int screen_w, screen_h;
    platform_init(&screen_w, &screen_h);

    Assets assets;
    assets_load(&assets);

    Game game;
    game_init(&game, 0); /* PSP build starts on map 0 (Forest); a map-select
                            screen is a natural next addition */

    double last_time = platform_time_seconds();

    while (1) {
        double now = platform_time_seconds();
        float dt = (float)(now - last_time);
        if (dt > 0.05f) dt = 0.05f;
        last_time = now;

        PlatformInput input;
        platform_poll_input(&input);

        int selected_before = game.selected_tower;
        int towers_before = game.towers.count;

        game_handle_input(&game, &input, dt);

        if (game.selected_tower != selected_before) platform_play_sound(assets.snd_select);
        if (game.towers.count != towers_before) platform_play_sound(assets.snd_place);

        game_update(&game, dt);
        if (game.shots_fired_this_frame > 0) platform_play_sound(assets.snd_shoot);

        render_game(&game, &assets);
    }

    return 0;
}
