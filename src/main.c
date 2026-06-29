/*
 * PS2 SMPTE 100% colour bars generator.
 *
 * Displays full-intensity SMPTE colour bars at every output mode the GS can
 * drive, so you can calibrate the per-channel gain of a RetroTink 4K (or any
 * scaler) at each resolution the PlayStation 2 can produce.
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

    gsKit_vram_clear(gsGlobal);
    gsKit_init_screen(gsGlobal);
    gsKit_mode_switch(gsGlobal, GS_ONESHOT);
    gsKit_set_test(gsGlobal, GS_ATEST_OFF);

    /* The font texture lives in VRAM, which we just cleared: re-upload it. */
    gsKit_fontm_upload(gsGlobal, gsFontM);
}

/* ------------------------------------------------------------------ frame */

static void draw_frame(int mode_idx, int level_idx, int show_hud)
{
    const VideoMode  *m   = &g_modes[mode_idx];
    const LevelPreset *lvl = &g_levels[level_idx];

    /* Clear to the floor level so any letterboxing matches black. */
    u64 floor_col = GS_SETREG_RGBAQ(lvl->floor, lvl->floor, lvl->floor, 0x80, 0x00);
    gsKit_clear(gsGlobal, floor_col);

    smpte_draw(gsGlobal, lvl);

    if (show_hud) {
        char line1[80];
        char line2[80];
        u64 hud_col = GS_SETREG_RGBAQ(0x40, 0x40, 0x40, 0x80, 0x00);

        snprintf(line1, sizeof(line1), "[%d/%d] %s",
                 mode_idx + 1, g_mode_count, m->name);
        snprintf(line2, sizeof(line2), "%s | %s (peak %d / floor %d)",
                 m->signal, lvl->name, lvl->peak, lvl->floor);

        gsKit_fontm_print_scaled(gsGlobal, gsFontM, 24.0f, 20.0f, 2, 0.5f, hud_col, line1);
        gsKit_fontm_print_scaled(gsGlobal, gsFontM, 24.0f, 48.0f, 2, 0.5f, hud_col, line2);
    }

    gsKit_queue_exec(gsGlobal);
    gsKit_sync_flip(gsGlobal);
}

/* ------------------------------------------------------------------- main */

int main(int argc, char *argv[])
{
    struct padButtonStatus buttons;
    u32 old_pad = 0;
    int mode_idx  = g_default_mode;
    int level_idx = 0;
    int show_hud  = 1;
    int redraw    = 2;   /* frames left to render (covers both buffers) */

    load_pad_modules();
    pad_init();

    /* GS + DMA bring-up. */
    gsGlobal = gsKit_init_global();
    dmaKit_init(D_CTRL_RELE_OFF, D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC,
                D_CTRL_STD_OFF, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);
    dmaKit_chan_init(DMA_CHANNEL_GIF);

    /* Embedded font: unpack once into RAM; upload happens per mode switch. */
    gsFontM = gsKit_init_fontm();
    gsKit_fontm_unpack(gsFontM);

    apply_mode(mode_idx);

    for (;;) {
        u32 pressed = 0;

        if (padRead(0, 0, &buttons) != 0) {
            u32 paddata = 0xffff ^ buttons.btns;  /* btns is active-low */
            pressed = paddata & ~old_pad;          /* rising edges only */
            old_pad = paddata;
        }

        if (pressed & (PAD_RIGHT | PAD_R1)) {
            mode_idx = (mode_idx + 1) % g_mode_count;
            apply_mode(mode_idx);
            redraw = 2;
        }
        if (pressed & (PAD_LEFT | PAD_L1)) {
            mode_idx = (mode_idx - 1 + g_mode_count) % g_mode_count;
            apply_mode(mode_idx);
            redraw = 2;
        }
        if (pressed & PAD_TRIANGLE) {              /* blind-recover to safe mode */
            mode_idx = g_default_mode;
            apply_mode(mode_idx);
            redraw = 2;
        }
        if (pressed & PAD_CROSS) {                 /* cycle level preset */
            level_idx = (level_idx + 1) % g_level_count;
            redraw = 2;
        }
        if (pressed & PAD_SELECT) {                /* toggle HUD text */
            show_hud = !show_hud;
            redraw = 2;
        }

        if (redraw > 0) {
            draw_frame(mode_idx, level_idx, show_hud);
            redraw--;
        }
    }

    return 0;
}
