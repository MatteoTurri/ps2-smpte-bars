#ifndef VIDEO_MODES_H
#define VIDEO_MODES_H

#include <gsKit.h>

/*
 * One selectable output mode for the GS CRTC.
 *
 * width/height are the *framebuffer* dimensions we render into. For solid
 * colour bars the exact vertical resolution is irrelevant (the bars are full
 * height), so interlaced modes that would normally flicker are rendered as a
 * single static field where convenient.
 *
 * psm:  GS_PSM_CT32 keeps true 8-bit-per-channel output (required for accurate
 *       gain calibration). A couple of very large modes do not fit in the 4 MB
 *       of GS VRAM at 32-bit and fall back to GS_PSM_CT16 (5 bits/channel ->
 *       peak white ~248, NOT 255). Those are flagged in their name.
 * dbuf: double buffering. Disabled for modes too large to hold two 32-bit
 *       buffers; harmless for a static image.
 */
typedef struct {
    const char *name;    /* short label shown in the HUD            */
    const char *signal;  /* cable / signal hint                     */
    s16  mode;           /* GS_MODE_*                               */
    s16  interlace;      /* GS_INTERLACED / GS_NONINTERLACED        */
    s16  field;          /* GS_FIELD / GS_FRAME                     */
    int  width;
    int  height;
    int  psm;            /* GS_PSM_CT32 / GS_PSM_CT16               */
    u8   dbuf;           /* GS_SETTING_ON / GS_SETTING_OFF          */
} VideoMode;

extern const VideoMode g_modes[];
extern const int g_mode_count;

/* Index of the default mode picked at boot (the most universally displayable
 * one: NTSC 480i over composite). */
extern const int g_default_mode;

#endif /* VIDEO_MODES_H */
