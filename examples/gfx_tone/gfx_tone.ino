// SPDX-FileCopyrightText: 2026 Adafruit Industries
// SPDX-License-Identifier: MIT
//
// Draws on a 320x240 canvas (shown at 640x480) while a 1 kHz tone plays
// over the same DVI cable. Needs a display or capture device that accepts
// audio; a plain DVI monitor shows only video.
// Leave Tools > CPU Speed at 150 MHz; the library sets 252 MHz itself.
#include <Adafruit_DVI_Audio.h>

Adafruit_DVI_Audio_GFX16 dvi;

#define AMPLITUDE 6000
static int16_t sine[48];  // one 1 kHz cycle at 48 kHz
static uint32_t phase;

static void feedAudio(void) {
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
}

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 48; i++) {
    sine[i] = (int16_t)(sinf(i * 2.0f * PI / 48) * AMPLITUDE);
  }
  if (!dvi.begin()) {
    for (;;) {
      Serial.println("DVI audio begin failed");
      delay(1000);
    }
  }
  dvi.fillScreen(0x0010);
  dvi.setTextColor(0xFFFF);
  dvi.setTextSize(2);
  dvi.setCursor(10, 10);
  dvi.println("Adafruit DVI Audio");
  dvi.setTextSize(1);
  dvi.setCursor(10, 34);
  dvi.println("1 kHz tone, 48 kHz stereo");
}

void setup1() {
  dvi.runCore1();
}

void loop() {
  feedAudio();

  static int x = 20, y = 60, dx = 2, dy = 1;
  static uint32_t last_frame;
  if (video_frame_count != last_frame) {  // move once per video frame
    last_frame = video_frame_count;
    dvi.fillRect(x, y, 24, 24, 0x0010);
    x += dx;
    y += dy;
    if (x <= 0 || x + 24 >= dvi.width()) dx = -dx;
    if (y <= 50 || y + 24 >= dvi.height()) dy = -dy;
    dvi.fillRect(x, y, 24, 24, 0xFFE0);
  }

  static uint32_t last;
  if (millis() - last >= 1000) {
    last = millis();
    dvi.fillRect(10, 224, 300, 8, 0x0010);
    dvi.setCursor(10, 224);
    dvi.printf("frames %lu  queue %lu", video_frame_count,
               hstx_di_queue_get_level());
    static uint32_t lf; Serial.printf("fps=%lu heap=%d sys=%lu frames=%lu queue=%lu\n", video_frame_count - lf, rp2040.getFreeHeap(), clock_get_hz(clk_sys),
                  video_frame_count, hstx_di_queue_get_level()); lf = video_frame_count;
  }
}
