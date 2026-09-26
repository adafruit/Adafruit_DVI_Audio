// SPDX-FileCopyrightText: 2026 Adafruit Industries
// SPDX-License-Identifier: MIT
//
// Colour bars plus a 1 kHz tone over one DVI cable. Needs a display or
// capture device that accepts audio; a plain DVI monitor shows only video.
// Leave Tools > CPU Speed at 150 MHz; the library sets 252 MHz itself.
#include <Adafruit_DVI_Audio.h>

Adafruit_DVI_Audio dvi;

static const uint16_t bars[8] = {0xFFFF, 0xFFE0, 0x07FF, 0x07E0,
                                 0xF81F, 0xF800, 0x001F, 0x0000};

static void __not_in_flash_func(scanline)(uint32_t v_scanline,
                                          uint32_t active_line,
                                          uint32_t *dst) {
  (void)v_scanline;
  (void)active_line;
  for (int bar = 0; bar < 8; bar++) {
    uint32_t c = bars[bar] | ((uint32_t)bars[bar] << 16);
    for (int i = 0; i < 40; i++) {  // 80 pixels per bar, 2 per word
      *dst++ = c;
    }
  }
}

#define TONE_HZ 1000
#define AMPLITUDE 6000
static int16_t sine[48];  // one 1 kHz cycle at 48 kHz
static uint32_t phase;

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 48; i++) {
    sine[i] = (int16_t)(sinf(i * 2.0f * PI / 48) * AMPLITUDE);
  }
  if (!dvi.begin(scanline, 48000)) {
    for (;;) {
      Serial.println("DVI audio begin failed");
      delay(1000);
    }
  }
}

void setup1() {
  dvi.runCore1();
}

void loop() {
  int16_t buf[64 * 2];
  size_t n = dvi.audioAvailableForWrite();
  if (n > 64) {
    n = 64;
  }
  n &= ~3u;
  for (size_t i = 0; i < n; i++) {
    int16_t s = sine[(phase + i) % 48];
    buf[i * 2] = s;
    buf[i * 2 + 1] = s;
  }
  phase = (phase + dvi.audioWrite(buf, n)) % 48;

  static uint32_t last;
  if (millis() - last >= 1000) {
    last = millis();
    static uint32_t lf; Serial.printf("fps=%lu heap=%d sys=%lu hstx=%lu frames=%lu queue=%lu\n", video_frame_count - lf, rp2040.getFreeHeap(),
                  clock_get_hz(clk_sys), clock_get_hz(clk_hstx),
                  video_frame_count, hstx_di_queue_get_level()); lf = video_frame_count;
  }
}
