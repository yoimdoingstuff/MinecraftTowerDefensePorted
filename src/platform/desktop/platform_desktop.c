#include "../../core/platform.h"
#include "../../core/font5x7.h"
#include "../../third_party/lodepng.h"

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCREEN_W 480
#define SCREEN_H 272
#define AUDIO_RATE 22050
#define MAX_VOICES 8

struct PlatformTexture { SDL_Texture *tex; int w, h; };
struct PlatformSound { int16_t *samples; Uint32 count; };

static SDL_Window *s_window = NULL;
static SDL_Renderer *s_renderer = NULL;
static int s_should_quit = 0;
static char s_asset_root[512] = "assets/";

typedef struct { const PlatformSound *snd; Uint32 pos; int active; } Voice;
static Voice s_voices[MAX_VOICES];
static SDL_AudioDeviceID s_audio_dev = 0;

static void audio_callback(void *userdata, Uint8 *stream, int len) {
    (void)userdata;
    memset(stream, 0, len);
    int16_t *out = (int16_t *)stream;
    int frames = len / 2;
    for (int v = 0; v < MAX_VOICES; v++) {
        if (!s_voices[v].active) continue;
        const PlatformSound *s = s_voices[v].snd;
        for (int i = 0; i < frames; i++) {
            if (s_voices[v].pos >= s->count) { s_voices[v].active = 0; break; }
            int32_t mixed = out[i] + s->samples[s_voices[v].pos++];
            if (mixed > 32767) mixed = 32767;
            if (mixed < -32768) mixed = -32768;
            out[i] = (int16_t)mixed;
        }
    }
}

static void resolve_asset_root(void) {
    /* Old approach guessed a handful of paths relative to the current
     * working directory - worked when launched via `./mtd2_desktop`
     * from inside its own folder, silently failed for any other
     * launch method (double-click, shortcut, launched from elsewhere),
     * since CWD isn't the executable's directory in those cases.
     * SDL_GetBasePath() returns the real directory containing the
     * executable on every platform, so this is correct regardless of
     * how/from-where the program was started. */
    char *base = SDL_GetBasePath();
    if (base) {
        char probe[700];
        snprintf(probe, sizeof probe, "%sassets/images/tile_path.png", base);
        FILE *f = fopen(probe, "rb");
        if (f) {
            fclose(f);
            snprintf(s_asset_root, sizeof s_asset_root, "%sassets/", base);
            SDL_free(base);
            return;
        }
        SDL_free(base);
    }
    /* Fall back to the old CWD-relative guesses (covers running
     * straight from the build directory during development). */
    static const char *candidates[] = {"assets/", "../assets/", "../../assets/"};
    for (size_t i = 0; i < sizeof(candidates) / sizeof(candidates[0]); i++) {
        char probe[600];
        snprintf(probe, sizeof probe, "%simages/tile_path.png", candidates[i]);
        FILE *f = fopen(probe, "rb");
        if (f) { fclose(f); strncpy(s_asset_root, candidates[i], sizeof(s_asset_root) - 1); return; }
    }
    fprintf(stderr, "[assets] could not locate the assets/ folder next to the executable "
                     "or in the working directory - textures/sounds will fail to load.\n");
}

void platform_init(int *out_screen_w, int *out_screen_h) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    s_window = SDL_CreateWindow("Minecraft Tower Defense 2 - PSP port (desktop build)",
                                 SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                 SCREEN_W * 2, SCREEN_H * 2, SDL_WINDOW_SHOWN);
    s_renderer = SDL_CreateRenderer(s_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_RenderSetLogicalSize(s_renderer, SCREEN_W, SCREEN_H);
    SDL_SetRenderDrawBlendMode(s_renderer, SDL_BLENDMODE_BLEND);

    strncpy(s_asset_root, "assets/", sizeof(s_asset_root) - 1);
    resolve_asset_root();

    SDL_AudioSpec want, have;
    memset(&want, 0, sizeof(want));
    want.freq = AUDIO_RATE;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 1024;
    want.callback = audio_callback;
    s_audio_dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (s_audio_dev) SDL_PauseAudioDevice(s_audio_dev, 0);

    *out_screen_w = SCREEN_W;
    *out_screen_h = SCREEN_H;
}

void platform_shutdown(void) {
    if (s_audio_dev) SDL_CloseAudioDevice(s_audio_dev);
    if (s_renderer) SDL_DestroyRenderer(s_renderer);
    if (s_window) SDL_DestroyWindow(s_window);
    SDL_Quit();
}

int platform_should_quit(void) { return s_should_quit; }

double platform_time_seconds(void) { return SDL_GetTicks64() / 1000.0; }

PlatformTexture *platform_load_texture(const char *relative_path) {
    char path[768];
    snprintf(path, sizeof path, "%s%s", s_asset_root, relative_path);

    unsigned char *pixels = NULL;
    unsigned w = 0, h = 0;
    unsigned err = lodepng_decode32_file(&pixels, &w, &h, path);
    if (err) {
        fprintf(stderr, "[assets] could not load %s: %s\n", path, lodepng_error_text(err));
        return NULL;
    }
    SDL_Surface *surf = SDL_CreateRGBSurfaceWithFormatFrom(
        pixels, (int)w, (int)h, 32, (int)w * 4, SDL_PIXELFORMAT_ABGR8888);
    SDL_Texture *tex = SDL_CreateTextureFromSurface(s_renderer, surf);
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    SDL_FreeSurface(surf);
    free(pixels);

    if (!tex) return NULL;
    PlatformTexture *pt = malloc(sizeof(PlatformTexture));
    pt->tex = tex;
    pt->w = (int)w;
    pt->h = (int)h;
    return pt;
}

void platform_free_texture(PlatformTexture *tex) {
    if (!tex) return;
    SDL_DestroyTexture(tex->tex);
    free(tex);
}

void platform_texture_size(const PlatformTexture *tex, int *w, int *h) {
    if (!tex) { *w = 0; *h = 0; return; }
    *w = tex->w; *h = tex->h;
}

void platform_clear(uint8_t r, uint8_t g, uint8_t b) {
    SDL_SetRenderDrawColor(s_renderer, r, g, b, 255);
    SDL_RenderClear(s_renderer);
}

void platform_draw_texture(const PlatformTexture *tex, int x, int y) {
    if (!tex) return;
    SDL_Rect dst = {x, y, tex->w, tex->h};
    SDL_RenderCopy(s_renderer, tex->tex, NULL, &dst);
}

void platform_draw_texture_tinted(const PlatformTexture *tex, int x, int y,
                                   uint8_t r, uint8_t g, uint8_t b) {
    if (!tex) return;
    SDL_SetTextureColorMod(tex->tex, r, g, b);
    SDL_Rect dst = {x, y, tex->w, tex->h};
    SDL_RenderCopy(s_renderer, tex->tex, NULL, &dst);
    SDL_SetTextureColorMod(tex->tex, 255, 255, 255);
}

void platform_draw_texture_ex(const PlatformTexture *tex,
                               int src_x, int src_y, int src_w, int src_h,
                               int dest_x, int dest_y, int dest_w, int dest_h) {
    if (!tex) return;
    SDL_Rect src = {src_x, src_y, src_w, src_h};
    SDL_Rect dst = {dest_x, dest_y, dest_w, dest_h};
    SDL_RenderCopy(s_renderer, tex->tex, &src, &dst);
}

void platform_draw_rect(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    SDL_SetRenderDrawColor(s_renderer, r, g, b, a);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(s_renderer, &rect);
}

void platform_draw_text(int x, int y, const char *text, uint8_t r, uint8_t g, uint8_t b) {
    const int scale = 2;
    SDL_SetRenderDrawColor(s_renderer, r, g, b, 255);
    int cx = x;
    for (const char *p = text; *p; p++) {
        const FontGlyph *glyph = NULL;
        for (int i = 0; i < FONT_GLYPH_COUNT; i++) {
            if (FONT_GLYPHS[i].ch == *p) { glyph = &FONT_GLYPHS[i]; break; }
        }
        if (glyph) {
            for (int row = 0; row < 7; row++) {
                uint8_t bits = glyph->rows[row];
                for (int col = 0; col < 5; col++) {
                    if (bits & (1 << (4 - col))) {
                        SDL_Rect px = {cx + col * scale, y + row * scale, scale, scale};
                        SDL_RenderFillRect(s_renderer, &px);
                    }
                }
            }
        }
        cx += 6 * scale;
    }
}

void platform_present(void) { SDL_RenderPresent(s_renderer); }

PlatformSound *platform_load_sound(const char *relative_path) {
    char path[768];
    snprintf(path, sizeof path, "%s%s", s_asset_root, relative_path);

    SDL_AudioSpec spec;
    Uint8 *buf = NULL;
    Uint32 len = 0;
    if (!SDL_LoadWAV(path, &spec, &buf, &len)) {
        fprintf(stderr, "[assets] could not load %s: %s\n", path, SDL_GetError());
        return NULL;
    }

    PlatformSound *snd = malloc(sizeof(PlatformSound));
    if (spec.freq == AUDIO_RATE && spec.format == AUDIO_S16SYS && spec.channels == 1) {
        snd->count = len / 2;
        snd->samples = malloc(len);
        memcpy(snd->samples, buf, len);
    } else {
        SDL_AudioCVT cvt;
        SDL_BuildAudioCVT(&cvt, spec.format, spec.channels, spec.freq,
                           AUDIO_S16SYS, 1, AUDIO_RATE);
        cvt.len = (int)len;
        cvt.buf = malloc((size_t)len * (size_t)cvt.len_mult);
        memcpy(cvt.buf, buf, len);
        SDL_ConvertAudio(&cvt);
        snd->count = (Uint32)cvt.len_cvt / 2;
        snd->samples = malloc((size_t)cvt.len_cvt);
        memcpy(snd->samples, cvt.buf, (size_t)cvt.len_cvt);
        free(cvt.buf);
    }
    SDL_FreeWAV(buf);
    return snd;
}

void platform_free_sound(PlatformSound *snd) {
    if (!snd) return;
    free(snd->samples);
    free(snd);
}

void platform_play_sound(const PlatformSound *snd) {
    if (!snd || !s_audio_dev) return;
    SDL_LockAudioDevice(s_audio_dev);
    for (int i = 0; i < MAX_VOICES; i++) {
        if (!s_voices[i].active) {
            s_voices[i].active = 1;
            s_voices[i].snd = snd;
            s_voices[i].pos = 0;
            break;
        }
    }
    SDL_UnlockAudioDevice(s_audio_dev);
}

/* ---- input: SDL keyboard, with our own repeat timing for the cursor ---- */

static double s_next_repeat_time = 0.0;
static int s_prev_confirm = 0, s_prev_cancel = 0, s_prev_next = 0, s_prev_prevt = 0;
static int s_prev_pause = 0, s_prev_speed = 0, s_prev_wave = 0;

void platform_poll_input(PlatformInput *input) {
    memset(input, 0, sizeof(*input));

    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_QUIT) s_should_quit = 1;
    }

    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    if (keys[SDL_SCANCODE_ESCAPE]) s_should_quit = 1;

    double now = platform_time_seconds();
    int want_x = (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) -
                 (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]);
    int want_y = (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S]) -
                 (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W]);
    static int had_dir = 0;
    if ((want_x || want_y)) {
        if (!had_dir || now >= s_next_repeat_time) {
            input->move_x = want_x > 0 ? 1 : (want_x < 0 ? -1 : 0);
            input->move_y = want_y > 0 ? 1 : (want_y < 0 ? -1 : 0);
            s_next_repeat_time = now + (had_dir ? 0.15 : 0.30);
            had_dir = 1;
        }
    } else {
        had_dir = 0;
    }

    int confirm = keys[SDL_SCANCODE_RETURN] || keys[SDL_SCANCODE_Z];
    int cancel = keys[SDL_SCANCODE_BACKSPACE] || keys[SDL_SCANCODE_X];
    int nextt = keys[SDL_SCANCODE_E] || keys[SDL_SCANCODE_RIGHTBRACKET];
    int prevt = keys[SDL_SCANCODE_Q] || keys[SDL_SCANCODE_LEFTBRACKET];
    int pause = keys[SDL_SCANCODE_SPACE];
    int speed = keys[SDL_SCANCODE_TAB];
    int wave = keys[SDL_SCANCODE_N];

    input->just_confirm = confirm && !s_prev_confirm;
    input->just_cancel = cancel && !s_prev_cancel;
    input->just_next_tower = nextt && !s_prev_next;
    input->just_prev_tower = prevt && !s_prev_prevt;
    input->just_pause = pause && !s_prev_pause;
    input->just_speed_toggle = speed && !s_prev_speed;
    input->just_next_wave = wave && !s_prev_wave;

    s_prev_confirm = confirm; s_prev_cancel = cancel;
    s_prev_next = nextt; s_prev_prevt = prevt;
    s_prev_pause = pause; s_prev_speed = speed; s_prev_wave = wave;
}
