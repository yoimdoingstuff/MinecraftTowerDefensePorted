#include "../../core/game.h"
#include "../../core/assets.h"
#include "../../core/render.h"
#include "../../core/map_data.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    int map_index = 0;
    if (argc > 1) map_index = atoi(argv[1]);

    int screen_w, screen_h;
    platform_init(&screen_w, &screen_h);

    Assets assets;
    assets_load(&assets);

    Game game;
    game_init(&game, map_index);


    printf("Loaded map: %s (%d waves)\n", MAP_DEFS[map_index].display_name,
           MAP_DEFS[map_index].total_waves);
    printf("Controls: arrows/WASD move, Enter/Z place, Backspace/X sell, "
           "Q/E cycle tower, Space pause, Tab speed, N next wave, Esc quit\n");

    double last_time = platform_time_seconds();

    while (!platform_should_quit()) {
        double now = platform_time_seconds();
        float dt = (float)(now - last_time);
        if (dt > 0.05f) dt = 0.05f; /* clamp huge pauses (e.g. window drag) */
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

    assets_free(&assets);
    platform_shutdown();
    return 0;
}
