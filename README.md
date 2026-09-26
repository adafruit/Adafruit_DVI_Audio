# Adafruit DVI Audio

DVI video plus digital audio over one cable on RP2350 boards with an HSTX
DVI port. Picture and sound both go to the display, so a TV's own speakers
play the audio. Built for emulators and demos on the Adafruit Fruit Jam.

This is a thin Arduino wrapper around
[fliperama86/pico_hdmi](https://github.com/fliperama86/pico_hdmi), vendored
unmodified in `src/pico_hdmi/` (tag v0.0.19).

## Supported

- Board: Adafruit Fruit Jam (RP2350B), arduino-pico core.
- Video: 640x480 at 60 Hz, from one of two classes:
  - `Adafruit_DVI_Audio_GFX16`: a 320x240 RGB565 Adafruit GFX canvas
    (150 KB), shown pixel-doubled.
  - `Adafruit_DVI_Audio`: no framebuffer; your scanline callback draws
    each line.
- Audio: 48 kHz, 16-bit stereo.

## Before you start

- **Leave Tools > CPU Speed at 150 MHz.** `begin()` sets 252 MHz itself.
  252 MHz is a multiple of 12 MHz and gives the exact 25.2 MHz pixel clock.
- **Core 1 belongs to the library.** Call `runCore1()` from `setup1()`; it
  never returns.
- **Use a display that plays sound from the cable.** A plain DVI monitor or
  a DVI adapter shows the picture but no audio. If you are at a desk monitor,
  the Fruit Jam's own 1/8" headphone jack is simpler for sound.

## Drawing on the canvas

```cpp
#include <Adafruit_DVI_Audio.h>

Adafruit_DVI_Audio_GFX16 dvi;   // 320x240 GFXcanvas16

void setup() {
  dvi.begin();            // 48 kHz
  dvi.fillScreen(0x0010);
  dvi.setCursor(10, 10);
  dvi.print("hello");
}

void setup1() {
  dvi.runCore1();
}

void loop() {
  // keep audio fed, see below
}
```

## Playing audio

Audio is queued as interleaved stereo frames (L, R, L, R ...). Keep the
queue topped up from `loop()`:

```cpp
int16_t buf[64 * 2];
size_t n = dvi.audioAvailableForWrite();  // frames that fit now
if (n > 64) n = 64;
// fill buf with n frames ...
size_t queued = dvi.audioWrite(buf, n);
```

`audioWrite()` sends frames in groups of 4 and returns how many it queued,
which can be less than asked for when the queue is full. Resend the rest
next time. When the queue runs dry the display gets silence, not a repeat.

## Emulators: your own scanline callback

Use `Adafruit_DVI_Audio`, which has no canvas, and generate each line
yourself:

```cpp
static void __not_in_flash_func(scanline)(uint32_t v_scanline,
                                          uint32_t active_line,
                                          uint32_t *dst) {
  // write 640 RGB565 pixels for output line active_line (0-479),
  // two pixels per 32-bit word: dst[0..319]
}

Adafruit_DVI_Audio dvi;

dvi.begin(scanline, 48000);
```

The callback runs on core 1 inside the video interrupt once per line, so it
must be fast and must live in RAM (`__not_in_flash_func`). No canvas is
allocated, which leaves about 150 KB more RAM than the GFX16 class.

## Examples

- `tone_bars` (`Adafruit_DVI_Audio`): colour bars and a 1 kHz tone from a
  scanline callback.
- `gfx_tone` (`Adafruit_DVI_Audio_GFX16`): canvas text and a bouncing box
  while a 1 kHz tone plays.

## Updating pico_hdmi

`tools/revendor.sh [commit]` copies a pico_hdmi commit into `src/pico_hdmi/`
without editing any file. Arduino does not run pico_hdmi's CMake, so its
build options are restated in `src/dvi_audio_config.h`; check them against
upstream's `CMakeLists.txt` after updating.

`src/dvi_video_output.c` moves one upstream function, `build_line_with_di`,
into RAM. It is called from the per-line video interrupt, and running it
from flash at 252 MHz could desync the video. Keep that declaration when
updating.

## License

Adafruit's code is MIT. pico_hdmi is The Unlicense. Part of pico_hdmi is
derived from Raspberry Pi's BSD-3-Clause HSTX example. See `NOTICE`.
