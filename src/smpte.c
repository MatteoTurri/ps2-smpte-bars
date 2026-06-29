#include "smpte.h"

const LevelPreset g_levels[] = {
    { "Full 0-255",    255,  0 },
    { "Studio 16-235", 235, 16 },
    { "240pTS 16-255", 255, 16 },
};

const int g_level_count = sizeof(g_levels) / sizeof(g_levels[0]);

/*
 * 100% SMPTE/EIA colour bars, left to right:
 *   white, yellow, cyan, green, magenta, red, blue
 * Each entry is the R/G/B on/off mask; "on" -> peak, "off" -> floor.
 */
static const unsigned char bar_bits[7][3] = {
    {1, 1, 1},  /* white   */
    {1, 1, 0},  /* yellow  */
    {0, 1, 1},  /* cyan    */
    {0, 1, 0},  /* green   */
    {1, 0, 1},  /* magenta */
    {1, 0, 0},  /* red     */
    {0, 0, 1},  /* blue    */
};

void smpte_draw(GSGLOBAL *gsGlobal, const LevelPreset *lvl)
{
    int w = gsGlobal->Width;
    int h = gsGlobal->Height;
    int i;

    for (i = 0; i < 7; i++) {
        int x1 = (w * i) / 7;
        int x2 = (w * (i + 1)) / 7;   /* last bar reaches exactly w */
        unsigned char r = bar_bits[i][0] ? lvl->peak : lvl->floor;
        unsigned char g = bar_bits[i][1] ? lvl->peak : lvl->floor;
        unsigned char b = bar_bits[i][2] ? lvl->peak : lvl->floor;

        /* alpha 0x80 == 1.0 in GS terms; q unused */
        u64 color = GS_SETREG_RGBAQ(r, g, b, 0x80, 0x00);
        gsKit_prim_sprite(gsGlobal, (float)x1, 0.0f, (float)x2, (float)h, 1, color);
    }
}
