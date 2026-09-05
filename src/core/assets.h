#ifndef MTD_ASSETS_H
#define MTD_ASSETS_H

#include "platform.h"
#include "tower.h"
#include "enemy.h"

typedef struct {
    PlatformTexture *tile_buildable;
    PlatformTexture *tile_path;
    PlatformTexture *cursor;
    PlatformTexture *projectile;
    PlatformTexture *tower_tex[TOWER_COUNT];
    PlatformTexture *enemy_tex[ENEMY_COUNT];

    PlatformSound *snd_place;   /* real extracted "pop" sfx */
    PlatformSound *snd_select;  /* real extracted "click" sfx */
    PlatformSound *snd_shoot;   /* real extracted "bow" sfx - used for every tower for now */
} Assets;

void assets_load(Assets *a);
void assets_free(Assets *a);

#endif /* MTD_ASSETS_H */
