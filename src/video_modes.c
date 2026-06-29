#include "video_modes.h"

/* Recommended/required cable for the mode. NOTE: this is a hint, not a probe:
 * the PS2 multi-AV connector carries composite, S-Video, RGB and YPbPr at the
 * same time on different pins, so what actually reaches the scaler depends on
 * the cable you plugged in. The software cannot detect that. */
#define SD_SIGNAL  "any cable"
#define HD_SIGNAL  "component"
#define VGA_SIGNAL "VGA RGBHV"

#define ON  GS_SETTING_ON
#define OFF GS_SETTING_OFF
#define I   GS_INTERLACED
#define NI  GS_NONINTERLACED
#define FRAME GS_FRAME
#define FIELD GS_FIELD

/*
 * Every output the PS2 Graphics Synthesizer can drive, grouped by signal type.
 *
 * 240p / 288p are produced natively by setting the SDTV modes non-interlaced
 * (no PS1 backwards-compatibility needed). 480i/576i use FIELD (gsKit's own
 * working default): with FRAME, gsKit does `MagV--` for interlaced modes, which
 * underflows our MagV to -1 and corrupts the vertical scaling.
 *
 * HD (480p..1080i) is YPbPr-only out of the multi-AV connector.
 * 1080i does not fit VRAM at 32-bit, so it is a single 1920x540 field at 16-bit.
 *
 * VGA/VESA modes are RGBHV; they need a display/scaler input that accepts
 * separate H/V sync (e.g. the RetroTink 4K analog port via the proper cable).
 */
const VideoMode g_modes[] = {
    /* --- SDTV ------------------------------------------------------------ */
    { "240p (NTSC)", SD_SIGNAL, GS_MODE_NTSC, NI, FRAME,  640, 224, GS_PSM_CT32, ON },
    { "288p (PAL)",  SD_SIGNAL, GS_MODE_PAL,  NI, FRAME,  640, 256, GS_PSM_CT32, ON },
    { "480i (NTSC)", SD_SIGNAL, GS_MODE_NTSC, I,  FIELD,  640, 448, GS_PSM_CT32, ON },
    { "576i (PAL)",  SD_SIGNAL, GS_MODE_PAL,  I,  FIELD,  640, 512, GS_PSM_CT32, ON },

    /* --- HD / DTV (component) ------------------------------------------- */
    { "480p",            HD_SIGNAL, GS_MODE_DTV_480P,  NI, FRAME,  640,  480, GS_PSM_CT32, ON  },
    { "576p",            HD_SIGNAL, GS_MODE_DTV_576P,  NI, FRAME,  640,  576, GS_PSM_CT32, ON  },
    { "720p",            HD_SIGNAL, GS_MODE_DTV_720P,  NI, FRAME, 1280,  720, GS_PSM_CT32, OFF },
    { "1080i (16-bit)",  HD_SIGNAL, GS_MODE_DTV_1080I, I,  FIELD, 1920,  540, GS_PSM_CT16, OFF },

    /* --- VGA / VESA (RGBHV) --------------------------------------------- */
    { "VGA 640x480 60Hz",   VGA_SIGNAL, GS_MODE_VGA_640_60,  NI, FRAME,  640,  480, GS_PSM_CT32, ON  },
    { "VGA 640x480 72Hz",   VGA_SIGNAL, GS_MODE_VGA_640_72,  NI, FRAME,  640,  480, GS_PSM_CT32, ON  },
    { "VGA 640x480 75Hz",   VGA_SIGNAL, GS_MODE_VGA_640_75,  NI, FRAME,  640,  480, GS_PSM_CT32, ON  },
    { "VGA 640x480 85Hz",   VGA_SIGNAL, GS_MODE_VGA_640_85,  NI, FRAME,  640,  480, GS_PSM_CT32, ON  },
    { "VGA 800x600 56Hz",   VGA_SIGNAL, GS_MODE_VGA_800_56,  NI, FRAME,  800,  600, GS_PSM_CT32, OFF },
    { "VGA 800x600 60Hz",   VGA_SIGNAL, GS_MODE_VGA_800_60,  NI, FRAME,  800,  600, GS_PSM_CT32, OFF },
    { "VGA 800x600 72Hz",   VGA_SIGNAL, GS_MODE_VGA_800_72,  NI, FRAME,  800,  600, GS_PSM_CT32, OFF },
    { "VGA 800x600 75Hz",   VGA_SIGNAL, GS_MODE_VGA_800_75,  NI, FRAME,  800,  600, GS_PSM_CT32, OFF },
    { "VGA 800x600 85Hz",   VGA_SIGNAL, GS_MODE_VGA_800_85,  NI, FRAME,  800,  600, GS_PSM_CT32, OFF },
    { "VGA 1024x768 60Hz",  VGA_SIGNAL, GS_MODE_VGA_1024_60, NI, FRAME, 1024,  768, GS_PSM_CT32, OFF },
    { "VGA 1024x768 70Hz",  VGA_SIGNAL, GS_MODE_VGA_1024_70, NI, FRAME, 1024,  768, GS_PSM_CT32, OFF },
    { "VGA 1024x768 75Hz",  VGA_SIGNAL, GS_MODE_VGA_1024_75, NI, FRAME, 1024,  768, GS_PSM_CT32, OFF },
    { "VGA 1024x768 85Hz",  VGA_SIGNAL, GS_MODE_VGA_1024_85, NI, FRAME, 1024,  768, GS_PSM_CT32, OFF },
    { "VGA 1280x1024 60Hz (16-bit)", VGA_SIGNAL, GS_MODE_VGA_1280_60, NI, FRAME, 1280, 1024, GS_PSM_CT16, OFF },
    { "VGA 1280x1024 75Hz (16-bit)", VGA_SIGNAL, GS_MODE_VGA_1280_75, NI, FRAME, 1280, 1024, GS_PSM_CT16, OFF },
};

const int g_mode_count = sizeof(g_modes) / sizeof(g_modes[0]);

/* NTSC 480i: works on virtually any display via composite. */
const int g_default_mode = 2;
