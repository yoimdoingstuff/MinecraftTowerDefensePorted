/* Platform abstraction layer.
 *
 * Core game code (grid/tower/enemy/wave/game) never includes a
 * platform-specific header and never calls a platform-specific API
 * directly. It only calls the functions declared here. Each backend
 * (src/platform/desktop, src/platform/psp) provides one .c file that
 * implements all of them.
 */
#ifndef MTD_PLATFORM_H
#define MTD_PLATFORM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- lifecycle ---------------------------------------------------- */

/* Sets up video/audio/input and reports the usable screen size in
 * pixels (480x272 on PSP; the desktop backend picks a matching window
 * size so the same tile-based layout works unmodified on both). */
void platform_init(int *out_screen_w, int *out_screen_h);
void platform_shutdown(void);

/* Returns 0 once the user has asked to quit (desktop window close;
 * PSP HOME button handling is backend-internal). */
int platform_should_quit(void);

/* Monotonic time in seconds, for frame timing. */
double platform_time_seconds(void);

/* ---- textures ------------------------------------------------------ */

typedef struct PlatformTexture PlatformTexture;

/* Loads a PNG from a path relative to the app's asset root and
 * uploads it in whatever form the backend's renderer wants. Returns
 * NULL on failure (missing file, decode error). */
PlatformTexture *platform_load_texture(const char *relative_path);
void platform_free_texture(PlatformTexture *tex);
void platform_texture_size(const PlatformTexture *tex, int *w, int *h);

/* ---- drawing --------------------------------------------------------
 * Origin is top-left, +x right, +y down, matching the tile grid. */

void platform_clear(uint8_t r, uint8_t g, uint8_t b);

void platform_draw_texture(const PlatformTexture *tex, int x, int y);

/* Draws a sub-rectangle of the texture (src_x,src_y,src_w,src_h) at
 * dest x,y sized dest_w,dest_h. Used for the 40x40 tile sheets and for
 * simple stretch-to-fit UI bars. */
void platform_draw_texture_ex(const PlatformTexture *tex,
                               int src_x, int src_y, int src_w, int src_h,
                               int dest_x, int dest_y, int dest_w, int dest_h);

void platform_draw_rect(int x, int y, int w, int h,
                         uint8_t r, uint8_t g, uint8_t b, uint8_t a);

/* Minimal built-in bitmap font (see font5x7.h) - no external asset. */
void platform_draw_text(int x, int y, const char *text,
                         uint8_t r, uint8_t g, uint8_t b);

void platform_present(void);

/* ---- audio ---------------------------------------------------------- */

typedef struct PlatformSound PlatformSound;

/* Loads a mono/stereo 16-bit PCM WAV relative to the asset root. */
PlatformSound *platform_load_sound(const char *relative_path);
void platform_free_sound(PlatformSound *snd);
void platform_play_sound(const PlatformSound *snd);

/* ---- input -----------------------------------------------------------
 * One frame's worth of input state. `just_*` fields are edge-triggered
 * (true for exactly the one frame the button transitioned to pressed)
 * so the core doesn't need to track previous state itself. */

typedef struct {
    /* cursor movement, already debounced/repeat-timed by the backend */
    int move_x;   /* -1, 0, +1 */
    int move_y;   /* -1, 0, +1 */

    int just_confirm;    /* PSP X / desktop Enter or Z */
    int just_cancel;     /* PSP O / desktop Backspace or X */
    int just_next_tower; /* PSP Triangle / desktop E or Right-bracket */
    int just_prev_tower; /* PSP Square / desktop Q or Left-bracket */
    int just_pause;      /* PSP Start / desktop Space */
    int just_speed_toggle;/* PSP L trigger / desktop Tab */
    int just_next_wave;  /* PSP R trigger / desktop N */
} PlatformInput;

void platform_poll_input(PlatformInput *input);

#ifdef __cplusplus
}
#endif

#endif /* MTD_PLATFORM_H */
