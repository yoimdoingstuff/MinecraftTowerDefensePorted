#include "../../core/platform.h"
#include "../../core/font5x7.h"
#include "../../third_party/lodepng.h"

#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspaudiolib.h>
#include <psputils.h>

#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_W 480
#define SCREEN_H 272
#define BUF_WIDTH 512

static unsigned int __attribute__((aligned(16))) s_display_list[262144];

struct PlatformTexture {
    void *pixels;    /* RGBA8888, pot_w x pot_h, 16-byte aligned */
    int pot_w, pot_h;
    int orig_w, orig_h;
};
struct PlatformSound {
    short *samples;  /* mono 16-bit */
    unsigned int count;
};

typedef struct { float u, v; float x, y, z; } TexVertex;
typedef struct { unsigned int color; float x, y, z; } ColorVertex;

/* ---- lifecycle / exit callback (standard PSP homebrew boilerplate) ---- */

static int exit_callback(int arg1, int arg2, void *common) {
    (void)arg1; (void)arg2; (void)common;
    sceKernelExitGame();
    return 0;
}
static int callback_thread(SceSize args, void *argp) {
    (void)args; (void)argp;
    int cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    sceKernelRegisterExitCallback(cbid);
    sceKernelSleepThreadCB();
    return 0;
}
static void setup_callbacks(void) {
    int thid = sceKernelCreateThread("update_thread", callback_thread, 0x11, 0xFA0, 0, NULL);
    if (thid >= 0) sceKernelStartThread(thid, 0, NULL);
}

PSP_MODULE_INFO("MTD2_PSP_PORT", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(-1024); /* leave 1MB for the OS, use the rest - plenty for our tiny assets */

static void *s_draw_buf = (void *)0;

void platform_init(int *out_screen_w, int *out_screen_h) {
    setup_callbacks();

    sceGuInit();
    sceGuStart(GU_DIRECT, s_display_list);
    sceGuDrawBuffer(GU_PSM_8888, (void *)0, BUF_WIDTH);
    sceGuDispBuffer(SCREEN_W, SCREEN_H, (void *)(BUF_WIDTH * SCREEN_H * 4), BUF_WIDTH);
    sceGuDepthRange(65535, 0);
    sceGuScissor(0, 0, SCREEN_W, SCREEN_H);
    sceGuEnable(GU_SCISSOR_TEST);
    sceGuFrontFace(GU_CW);
    sceGuShadeModel(GU_SMOOTH);
    sceGuEnable(GU_TEXTURE_2D);
    sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);
    sceGuTexFilter(GU_NEAREST, GU_NEAREST);
    sceGuTexWrap(GU_CLAMP, GU_CLAMP);
    sceGuEnable(GU_BLEND);
    sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
    sceGuFinish();
    sceGuSync(0, 0);

    sceDisplayWaitVblankStart();
    sceGuDisplay(GU_TRUE);

    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

    pspAudioInit();

    *out_screen_w = SCREEN_W;
    *out_screen_h = SCREEN_H;
}

void platform_shutdown(void) {
    pspAudioEnd();
    sceGuTerm();
}

int platform_should_quit(void) { return 0; /* HOME button exits via callback thread, see above */ }

double platform_time_seconds(void) { return (double)sceKernelGetSystemTimeWide() / 1000000.0; }

/* ---- textures: pad to power-of-two so the GE can sample them ---- */

static int next_pot(int v) { int p = 1; while (p < v) p <<= 1; return p; }

static const char *s_asset_root = "assets/";

PlatformTexture *platform_load_texture(const char *relative_path) {
    char path[512];
    snprintf(path, sizeof path, "%s%s", s_asset_root, relative_path);

    unsigned char *rgba = NULL;
    unsigned w = 0, h = 0;
    unsigned err = lodepng_decode32_file(&rgba, &w, &h, path);
    if (err) return NULL;

    int pot_w = next_pot((int)w), pot_h = next_pot((int)h);
    unsigned int *buf = memalign(16, (size_t)pot_w * pot_h * 4);
    memset(buf, 0, (size_t)pot_w * pot_h * 4);
    for (unsigned y = 0; y < h; y++) {
        memcpy((unsigned char *)buf + (size_t)y * pot_w * 4,
               rgba + (size_t)y * w * 4, w * 4);
    }
    free(rgba);
    sceKernelDcacheWritebackInvalidateRange(buf, (unsigned int)(pot_w * pot_h * 4));

    PlatformTexture *tex = malloc(sizeof(PlatformTexture));
    tex->pixels = buf;
    tex->pot_w = pot_w; tex->pot_h = pot_h;
    tex->orig_w = (int)w; tex->orig_h = (int)h;
    return tex;
}

void platform_free_texture(PlatformTexture *tex) {
    if (!tex) return;
    free(tex->pixels);
    free(tex);
}

void platform_texture_size(const PlatformTexture *tex, int *w, int *h) {
    if (!tex) { *w = 0; *h = 0; return; }
    *w = tex->orig_w; *h = tex->orig_h;
}

/* ---- drawing ---- */

void platform_clear(uint8_t r, uint8_t g, uint8_t b) {
    sceGuClearColor(GU_ABGR(255, b, g, r));
    sceGuClear(GU_COLOR_BUFFER_BIT);
}

static void bind_texture(const PlatformTexture *tex) {
    sceGuTexMode(GU_PSM_8888, 0, 0, 0);
    sceGuTexImage(0, tex->pot_w, tex->pot_h, tex->pot_w, tex->pixels);
}

static void draw_tex_quad(const PlatformTexture *tex,
                           float su0, float sv0, float su1, float sv1,
                           int dx, int dy, int dw, int dh) {
    TexVertex *v = sceGuGetMemory(2 * sizeof(TexVertex));
    v[0].u = su0 / tex->pot_w; v[0].v = sv0 / tex->pot_h;
    v[0].x = (float)dx; v[0].y = (float)dy; v[0].z = 0;
    v[1].u = su1 / tex->pot_w; v[1].v = sv1 / tex->pot_h;
    v[1].x = (float)(dx + dw); v[1].y = (float)(dy + dh); v[1].z = 0;
    bind_texture(tex);
    sceGuDrawArray(GU_SPRITES, GU_TEXTURE_32BITF | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, v);
}

void platform_draw_texture(const PlatformTexture *tex, int x, int y) {
    if (!tex) return;
    draw_tex_quad(tex, 0, 0, (float)tex->orig_w, (float)tex->orig_h, x, y, tex->orig_w, tex->orig_h);
}

void platform_draw_texture_ex(const PlatformTexture *tex,
                               int src_x, int src_y, int src_w, int src_h,
                               int dest_x, int dest_y, int dest_w, int dest_h) {
    if (!tex) return;
    draw_tex_quad(tex, (float)src_x, (float)src_y, (float)(src_x + src_w), (float)(src_y + src_h),
                  dest_x, dest_y, dest_w, dest_h);
}

void platform_draw_rect(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    sceGuDisable(GU_TEXTURE_2D);
    ColorVertex *v = sceGuGetMemory(2 * sizeof(ColorVertex));
    unsigned int color = GU_ABGR(a, b, g, r);
    v[0].color = color; v[0].x = (float)x;     v[0].y = (float)y;     v[0].z = 0;
    v[1].color = color; v[1].x = (float)(x+w); v[1].y = (float)(y+h); v[1].z = 0;
    sceGuDrawArray(GU_SPRITES, GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, v);
    sceGuEnable(GU_TEXTURE_2D);
}

void platform_draw_text(int x, int y, const char *text, uint8_t r, uint8_t g, uint8_t b) {
    const int scale = 2;
    sceGuDisable(GU_TEXTURE_2D);
    unsigned int color = GU_ABGR(255, b, g, r);
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
                    if (!(bits & (1 << (4 - col)))) continue;
                    ColorVertex *v = sceGuGetMemory(2 * sizeof(ColorVertex));
                    int px = cx + col * scale, py = y + row * scale;
                    v[0].color = color; v[0].x = (float)px; v[0].y = (float)py; v[0].z = 0;
                    v[1].color = color; v[1].x = (float)(px+scale); v[1].y = (float)(py+scale); v[1].z = 0;
                    sceGuDrawArray(GU_SPRITES, GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, v);
                }
            }
        }
        cx += 6 * scale;
    }
    sceGuEnable(GU_TEXTURE_2D);
}

void platform_present(void) {
    sceGuFinish();
    sceGuSync(0, 0);
    sceDisplayWaitVblankStart();
    s_draw_buf = sceGuSwapBuffers();
    sceGuStart(GU_DIRECT, s_display_list);
}

/* ---- audio: pspaudiolib callback mixer, mirrors the desktop backend ---- */

#define MAX_VOICES 8
typedef struct { const PlatformSound *snd; unsigned int pos; int active; } Voice;
static Voice s_voices[MAX_VOICES];

/* pspaudiolib's default reserved channel format is stereo 16-bit;
 * `reqn` is frame count (2 int16s per frame). Our source sounds are
 * mono, so each mixed sample is written to both L and R. */
static void audio_callback(void *buf, unsigned int reqn, void *pdata) {
    (void)pdata;
    short *out = (short *)buf;
    memset(out, 0, reqn * 2 * sizeof(short));
    for (int v = 0; v < MAX_VOICES; v++) {
        if (!s_voices[v].active) continue;
        const PlatformSound *s = s_voices[v].snd;
        for (unsigned int i = 0; i < reqn; i++) {
            if (s_voices[v].pos >= s->count) { s_voices[v].active = 0; break; }
            int sample = s->samples[s_voices[v].pos++];
            int l = out[i*2+0] + sample; if (l > 32767) l = 32767; if (l < -32768) l = -32768;
            int r = out[i*2+1] + sample; if (r > 32767) r = 32767; if (r < -32768) r = -32768;
            out[i*2+0] = (short)l; out[i*2+1] = (short)r;
        }
    }
}

static int s_audio_ready = 0;

PlatformSound *platform_load_sound(const char *relative_path) {
    char path[512];
    snprintf(path, sizeof path, "%s%s", s_asset_root, relative_path);

    if (!s_audio_ready) {
        pspAudioSetChannelCallback(0, audio_callback, NULL);
        s_audio_ready = 1;
    }

    /* Minimal WAV reader: our assets are always pre-converted to
     * mono 16-bit PCM (see assets/audio_wav and tools/), so this
     * doesn't need to handle arbitrary WAV variants - just enough to
     * pull the data chunk out of the files this project ships. */
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    unsigned char hdr[44];
    if (fread(hdr, 1, 44, f) != 44) { fclose(f); return NULL; }
    unsigned int data_size = hdr[40] | (hdr[41] << 8) | (hdr[42] << 16) | (hdr[43] << 24);

    PlatformSound *snd = malloc(sizeof(PlatformSound));
    snd->samples = malloc(data_size);
    fread(snd->samples, 1, data_size, f);
    fclose(f);
    snd->count = data_size / 2;
    return snd;
}

void platform_free_sound(PlatformSound *snd) {
    if (!snd) return;
    free(snd->samples);
    free(snd);
}

void platform_play_sound(const PlatformSound *snd) {
    if (!snd) return;
    for (int i = 0; i < MAX_VOICES; i++) {
        if (!s_voices[i].active) {
            s_voices[i].snd = snd;
            s_voices[i].pos = 0;
            s_voices[i].active = 1;
            break;
        }
    }
}

/* ---- input ---- */

static int s_prev_confirm=0, s_prev_cancel=0, s_prev_next=0, s_prev_prevt=0;
static int s_prev_pause=0, s_prev_speed=0, s_prev_wave=0;
static double s_next_repeat_time = 0.0;
static int s_had_dir = 0;

void platform_poll_input(PlatformInput *input) {
    memset(input, 0, sizeof(*input));

    SceCtrlData pad;
    sceCtrlReadBufferPositive(&pad, 1);

    int want_x = 0, want_y = 0;
    if (pad.Lx < 80 || (pad.Buttons & PSP_CTRL_LEFT)) want_x = -1;
    else if (pad.Lx > 176 || (pad.Buttons & PSP_CTRL_RIGHT)) want_x = 1;
    if (pad.Ly < 80 || (pad.Buttons & PSP_CTRL_UP)) want_y = -1;
    else if (pad.Ly > 176 || (pad.Buttons & PSP_CTRL_DOWN)) want_y = 1;

    double now = platform_time_seconds();
    if (want_x || want_y) {
        if (!s_had_dir || now >= s_next_repeat_time) {
            input->move_x = want_x; input->move_y = want_y;
            s_next_repeat_time = now + (s_had_dir ? 0.15 : 0.30);
            s_had_dir = 1;
        }
    } else {
        s_had_dir = 0;
    }

    int confirm = (pad.Buttons & PSP_CTRL_CROSS) != 0;
    int cancel = (pad.Buttons & PSP_CTRL_CIRCLE) != 0;
    int nextt = (pad.Buttons & PSP_CTRL_TRIANGLE) != 0;
    int prevt = (pad.Buttons & PSP_CTRL_SQUARE) != 0;
    int pause = (pad.Buttons & PSP_CTRL_START) != 0;
    int speed = (pad.Buttons & PSP_CTRL_LTRIGGER) != 0;
    int wave = (pad.Buttons & PSP_CTRL_RTRIGGER) != 0;

    input->just_confirm = confirm && !s_prev_confirm;
    input->just_cancel = cancel && !s_prev_cancel;
    input->just_next_tower = nextt && !s_prev_next;
    input->just_prev_tower = prevt && !s_prev_prevt;
    input->just_pause = pause && !s_prev_pause;
    input->just_speed_toggle = speed && !s_prev_speed;
    input->just_next_wave = wave && !s_prev_wave;

    s_prev_confirm=confirm; s_prev_cancel=cancel; s_prev_next=nextt; s_prev_prevt=prevt;
    s_prev_pause=pause; s_prev_speed=speed; s_prev_wave=wave;
}
