#ifndef SMPTE_H
#define SMPTE_H

#include <gsKit.h>

/*
 * A black/white level pair. SMPTE "100%" bars use full-saturation primaries,
 * so each of the 7 bars is just R/G/B switched between "peak" and "floor".
 *
 *  - Full 0-255   : PC / full range. True peak white = 255.
 *  - Studio 16-235: limited range as broadcast gear expects.
 *  - 240pTS 16-255: matches the 240p Test Suite note (black floor at 16,
 *                   colour peak at 255).
 *
 * Toggle between them to verify your RetroTink 4K gain across the range.
 */
typedef struct {
    const char *name;
    int peak;   /* 0-255 */
    int floor;  /* 0-255 */
} LevelPreset;

extern const LevelPreset g_levels[];
extern const int g_level_count;

/* Draw the 7 vertical 100% SMPTE bars filling the whole active framebuffer. */
void smpte_draw(GSGLOBAL *gsGlobal, const LevelPreset *lvl);

#endif /* SMPTE_H */
