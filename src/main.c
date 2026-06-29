/*
 * PS2 SMPTE 100% colour bars generator.
 *
 * Displays full-intensity SMPTE colour bars at every output mode the GS can
 * drive, so you can calibrate the per-channel gain of a RetroTink 4K (or any
 * scaler) at each resolution the PlayStation 2 can produce.
 *
 * Two views:
 *   MENU  - scrollable list to pick a resolution.
 *   BARS  - the colour bars in the selected mode, with an optional HUD.
 */
#include <stdio.h>
#include <string.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <libpad.h>

#include <gsKit.h>
#include <dmaKit.h>
#include <gsToolkit.h>
#include <gsFontM.h>

#include "video_modes.h"
#include "smpte.h"

/* FontM renders a fixed 26x26 monospace cell at scale 1.0: both the per-glyph
 * X advance and the line height are 26*scale pixels. We size text by *width*
 * (how many columns must fit), so a line of N characters always spans the same
 * fraction of the screen in every resolution and can never overflow the edge. */
#define FONTM_CELL 26.0f
#define MENU_COLS  40.0f   /* design width: 40 chars span ~92% of the screen */
#define HUD_COLS   44.0f
#define TEXT_MARGIN 0.04f  /* left/right margin as a fraction of width        */

enum { VIEW_MENU, VIEW_BARS };

static GSGLOBAL *gsGlobal;
static GSFONTM  *gsFontM;

/* libpad working buffer: 256 bytes, 64-byte aligned. */
static char pad_buf[256] __attribute__((aligned(64)));

/* ------------------------------------------------------------------ pads */

static void load_pad_modules(void)
{
    SifInitRpc(0);
    SifLoadModule("rom0:SIO2MAN", 0, NULL);
    SifLoadModule("rom0:PADMAN", 0, NULL);
}

static void wait_pad_ready(void)
{
    int state;
    do {
        state = padGetState(0, 0);
    } while (state != PAD_STATE_STABLE && state != PAD_STATE_FINDCTP1);
}

static void pad_init(void)
{
    padInit(0);
    padPortOpen(0, 0, pad_buf);
    wait_pad_ready();
}

/* ----------------------------------------------------------------- video */

/* (Re)configure the GS for the given mode index and prepare it for drawing. */
static void apply_mode(int idx)
{
    const VideoMode *m = &g_modes[idx];

    gsGlobal->Mode            = m->mode;
    gsGlobal->Interlace       = m->interlace;
    gsGlobal->Field           = m->field;
    gsGlobal->Width           = m->width;
    gsGlobal->Height          = m->height;
    gsGlobal->PSM             = m->psm;
    gsGlobal->PSMZ            = GS_PSMZ_16;       /* unused, Z disabled */
    gsGlobal->ZBuffering      = GS_SETTING_OFF;
    gsGlobal->DoubleBuffering = m->dbuf;
    gsGlobal->Dithering       = GS_SETTING_OFF;   /* exact, deterministic levels */
    gsGlobal->PrimAlphaEnable = GS_SETTING_OFF;   /* no blending: write colors verbatim */
    gsGlobal->PrimAAEnable    = GS_SETTING_OFF;   /* no AA on bar edges */

    gsKit_vram_clear(gsGlobal);
    gsKit_init_screen(gsGlobal);
    gsKit_mode_switch(gsGlobal, GS_ONESHOT);
    gsKit_set_test(gsGlobal, GS_ATEST_OFF);

    /* The font texture lives in VRAM, which we just cleared: re-upload it. */
    gsKit_fontm_upload(gsGlobal, gsFontM);
}

/* Text scale so that `cols` characters fit within the usable width. */
static float scale_for_cols(float cols)
{
    float usable = gsGlobal->Width * (1.0f - 2.0f * TEXT_MARGIN);
    return usable / (FONTM_CELL * cols);
}

static float menu_line_h(void)
{
    return FONTM_CELL * scale_for_cols(MENU_COLS);
}

/* How many menu rows fit on screen in the current mode. */
static int menu_visible(void)
{
    float top = gsGlobal->Height * 0.18f;
    float bot = gsGlobal->Height * 0.90f;
    int v = (int)((bot - top) / menu_line_h());
    if (v < 1) v = 1;
    if (v > g_mode_count) v = g_mode_count;
    return v;
}

/* ------------------------------------------------------------------ views */

static void draw_menu(int cursor, int active, int level_idx)
{
    int   W = gsGlobal->Width;
    int   H = gsGlobal->Height;
    float x       = W * TEXT_MARGIN;
    float scale   = scale_for_cols(MENU_COLS);
    float lineH   = menu_line_h();
    float listTop = H * 0.18f;
    int   visible = menu_visible();
    int   start, i;

    u64 cBg    = GS_SETREG_RGBAQ(0x00, 0x00, 0x00, 0x80, 0x00);
    u64 cTitle = GS_SETREG_RGBAQ(0x80, 0xC0, 0xFF, 0x80, 0x00);
    u64 cDim   = GS_SETREG_RGBAQ(0x70, 0x70, 0x70, 0x80, 0x00);
    u64 cSel   = GS_SETREG_RGBAQ(0xFF, 0xFF, 0xFF, 0x80, 0x00);
    u64 cFoot  = GS_SETREG_RGBAQ(0x55, 0x55, 0x55, 0x80, 0x00);

    start = cursor - visible / 2;
    if (start > g_mode_count - visible) start = g_mode_count - visible;
    if (start < 0) start = 0;

    gsKit_clear(gsGlobal, cBg);

    float ax = W * (1.0f - TEXT_MARGIN) - FONTM_CELL * scale; /* scroll arrow x */

    gsKit_fontm_print_scaled(gsGlobal, gsFontM, x, H * 0.06f, 2,
                             scale, cTitle, "SMPTE 100% Color Bars");

    for (i = 0; i < visible; i++) {
        int  idx = start + i;
        char row[64];
        char cur  = (idx == cursor) ? '>' : ' ';
        char mark = (idx == active) ? '*' : ' ';
        u64  col  = (idx == cursor) ? cSel : cDim;

        snprintf(row, sizeof(row), "%c%c %s", cur, mark, g_modes[idx].name);
        gsKit_fontm_print_scaled(gsGlobal, gsFontM, x,
                                 listTop + i * lineH, 2, scale, col, row);
    }

    if (start > 0)
        gsKit_fontm_print_scaled(gsGlobal, gsFontM, ax, listTop, 2,
                                 scale, cDim, "^");
    if (start + visible < g_mode_count)
        gsKit_fontm_print_scaled(gsGlobal, gsFontM, ax,
                                 listTop + (visible - 1) * lineH, 2, scale, cDim, "v");

    gsKit_fontm_print_scaled(gsGlobal, gsFontM, x, H * 0.93f, 2,
                             scale, cFoot, "Up/Down move  L1/R1 page  X select");

    gsKit_queue_exec(gsGlobal);
    gsKit_sync_flip(gsGlobal);
}

static void draw_bars(int active, int level_idx, int show_hud)
{
    const VideoMode   *m   = &g_modes[active];
    const LevelPreset *lvl = &g_levels[level_idx];
    int W = gsGlobal->Width;
    int H = gsGlobal->Height;

    u64 floor_col = GS_SETREG_RGBAQ(lvl->floor, lvl->floor, lvl->floor, 0x80, 0x00);
    gsKit_clear(gsGlobal, floor_col);

    smpte_draw(gsGlobal, lvl);

    if (show_hud) {
        float scale = scale_for_cols(HUD_COLS);
        float lineH = FONTM_CELL * scale;
        float x     = W * TEXT_MARGIN;
        float y     = H * 0.05f;
        u64   col   = GS_SETREG_RGBAQ(0x60, 0x60, 0x60, 0x80, 0x00);
        char  l1[64], l2[64];

        snprintf(l1, sizeof(l1), "%s  (%s)", m->name, m->signal);
        snprintf(l2, sizeof(l2), "%s   O=menu", lvl->name);

        gsKit_fontm_print_scaled(gsGlobal, gsFontM, x, y, 2, scale, col, l1);
        gsKit_fontm_print_scaled(gsGlobal, gsFontM, x, y + lineH, 2, scale, col, l2);
    }

    gsKit_queue_exec(gsGlobal);
    gsKit_sync_flip(gsGlobal);
}

/* ------------------------------------------------------------------- main */

int main(int argc, char *argv[])
{
    struct padButtonStatus buttons;
    u32 old_pad = 0;
    int view      = VIEW_MENU;
    int active    = g_default_mode;   /* currently applied GS mode  */
    int cursor    = g_default_mode;   /* menu highlight             */
    int level_idx = 0;
    int show_hud  = 1;
    int redraw    = 2;                /* frames left to draw (both buffers) */

    load_pad_modules();
    pad_init();

    gsGlobal = gsKit_init_global();
    dmaKit_init(D_CTRL_RELE_OFF, D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC,
                D_CTRL_STD_OFF, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);
    dmaKit_chan_init(DMA_CHANNEL_GIF);

    gsFontM = gsKit_init_fontm();
    gsKit_fontm_unpack(gsFontM);

    apply_mode(active);

    for (;;) {
        u32 pressed = 0;

        if (padRead(0, 0, &buttons) != 0) {
            u32 paddata = 0xffff ^ buttons.btns;  /* btns is active-low */
            pressed = paddata & ~old_pad;          /* rising edges only */
            old_pad = paddata;
        }

        if (view == VIEW_MENU) {
            int page = menu_visible();

            if (pressed & PAD_DOWN) {
                cursor = (cursor + 1) % g_mode_count;
                redraw = 2;
            }
            if (pressed & PAD_UP) {
                cursor = (cursor - 1 + g_mode_count) % g_mode_count;
                redraw = 2;
            }
            if (pressed & PAD_R1) {
                cursor += page;
                if (cursor > g_mode_count - 1) cursor = g_mode_count - 1;
                redraw = 2;
            }
            if (pressed & PAD_L1) {
                cursor -= page;
                if (cursor < 0) cursor = 0;
                redraw = 2;
            }
            if (pressed & PAD_CROSS) {              /* apply selection */
                if (cursor != active) {
                    active = cursor;
                    apply_mode(active);
                }
                view = VIEW_BARS;
                redraw = 2;
            }
            if (pressed & PAD_TRIANGLE) {           /* blind-recover safe mode */
                active = g_default_mode;
                cursor = active;
                apply_mode(active);
                view = VIEW_BARS;
                redraw = 2;
            }
        } else { /* VIEW_BARS */
            if (pressed & PAD_CROSS) {              /* cycle level preset */
                level_idx = (level_idx + 1) % g_level_count;
                redraw = 2;
            }
            if (pressed & PAD_SELECT) {             /* toggle HUD */
                show_hud = !show_hud;
                redraw = 2;
            }
            if (pressed & (PAD_CIRCLE | PAD_START)) { /* back to menu */
                cursor = active;
                view = VIEW_MENU;
                redraw = 2;
            }
            if (pressed & PAD_TRIANGLE) {           /* blind-recover safe mode */
                if (active != g_default_mode) {
                    active = g_default_mode;
                    apply_mode(active);
                }
                cursor = active;
                redraw = 2;
            }
        }

        if (redraw > 0) {
            if (view == VIEW_MENU)
                draw_menu(cursor, active, level_idx);
            else
                draw_bars(active, level_idx, show_hud);
            redraw--;
        } else {
            /* Nothing changed: just pace at vsync. Both buffers already hold
             * identical content, so this flip shows no visible change. */
            gsKit_sync_flip(gsGlobal);
        }
    }

    return 0;
}
