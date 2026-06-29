# PS2 SMPTE Color Bars

A **100% SMPTE color bars** generator for the PlayStation 2, built to
**calibrate the gain of a RetroTink 4K** (or any scaler / video processor) at
*every* resolution the PS2 Graphics Synthesizer can output.

The 7 full-intensity vertical bars (white, yellow, cyan, green, magenta, red,
blue) are the same reference pattern used by the
[240p Test Suite](https://github.com/ArtemioUrbina/240pTestSuite) in its 100%
variant.

## Supported resolutions

Selectable at runtime from a menu (see *Controls*):

| Group | Modes | Required output |
|---|---|---|
| SDTV | 240p, 288p, 480i, 576i | Composite / S-Video / RGB / Component |
| HD/DTV | 480p, 576p, 720p, 1080i | **Component (YPbPr)** |
| VGA/VESA | 640×480, 800×600, 1024×768 (60–85 Hz), 1280×1024 (60/75 Hz) | **VGA RGBHV** |

Notes:
- **240p / 288p** are produced natively by setting the SDTV modes
  non-interlaced — no PS1 backwards-compatibility needed.
- **HD modes are YPbPr-only** out of the multi-AV connector.
- **VGA modes** are RGBHV (separate H/V sync): they need a suitable cable and an
  input that accepts them (e.g. the RetroTink 4K analog port).

### Color precision (matters for calibration)

Almost every mode uses a **32-bit framebuffer (8 bits/channel)**, so peak white
is exactly **255** and the levels are accurate.

Two modes do not fit in the GS's 4 MB of VRAM at 32-bit and fall back to
**16-bit (5 bits/channel)** — peak white becomes ~248, not 255. They are tagged
`(16-bit)` in the menu:

- `1080i` (rendered as a single 1920×540 field)
- `VGA 1280×1024`

For fine gain calibration prefer the other modes; these two are still useful for
checking sync/geometry.

## Controls

It boots into a **menu** listing every resolution; pick one to show the bars.

**Menu**

| Button | Action |
|---|---|
| D-Pad ▲ / ▼ | Move selection |
| L1 / R1 | Page up / down |
| ✕ (Cross) | Apply selected resolution → bars |
| △ (Triangle) | Jump to the safe **480i NTSC** mode (blind recovery) |

**Bars**

| Button | Action |
|---|---|
| ✕ (Cross) | Cycle level preset |
| Select | Show/hide the text HUD |
| ○ (Circle) / Start | Back to the menu |
| △ (Triangle) | Jump back to the safe **480i NTSC** mode (blind recovery) |

### Level presets

| Preset | Floor (black) | Peak (white / primaries) |
|---|---|---|
| Full 0-255 | 0 | 255 |
| Studio 16-235 | 16 | 235 |
| 240pTS 16-255 | 16 | 255 |

The HUD in the top-left shows the current mode, signal type, and active preset.
Hide it with **Select** for a clean measurement.

> If you switch to a mode your display/scaler can't sync and lose the picture,
> press **△** to return to 480i NTSC.

## Build

Only **Docker** is required (uses the official `ps2dev/ps2dev` image, nothing to
install on the host):

```sh
./build.sh          # produces smpte-bars.elf
./build.sh clean    # cleanup
```

Alternatively, with the ps2dev toolchain installed natively (`$PS2SDK` and
`$GSKIT` exported):

```sh
make
```

## Running

- **Emulator:** load `smpte-bars.elf` in [PCSX2](https://pcsx2.net/) (handy for
  development; real calibration must be done on actual hardware).
- **Real hardware:** launch the `.elf` via uLaunchELF (USB/MC), PS2Link/ps2client
  over the network, or an equivalent loader.

## Layout

```
src/
  main.c          # GS/DMA/pad init, input loop, rendering
  video_modes.c   # table of every GS output mode
  smpte.c         # 7-bar pattern + level presets
Makefile          # EE build (PS2SDK + gsKit)
build.sh          # Docker wrapper for reproducible builds
```

## License

MIT — see [LICENSE](LICENSE).
