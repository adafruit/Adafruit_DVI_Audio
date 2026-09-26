// SPDX-FileCopyrightText: 2026 Adafruit Industries
// SPDX-License-Identifier: MIT
//
// DVI video plus digital audio on RP2350 HSTX, wrapping fliperama86/pico_hdmi.
#pragma once

#include <Adafruit_GFX.h>
#include <Arduino.h>

#include "dvi_audio_config.h"
// Upstream headers have no extern "C" guards of their own.
extern "C" {
#include "pico_hdmi/hstx_data_island_queue.h"
#include "pico_hdmi/hstx_packet.h"
#include "pico_hdmi/video_output.h"
}

// DVI video from a per-line callback, plus audio. No framebuffer.
class Adafruit_DVI_Audio {
public:
  // Called on core 1 for every active line (active_line 0-479). line_buffer
  // takes 640 RGB565 pixels packed two per word. Must be fast; keep it in
  // RAM with __not_in_flash_func.
  typedef video_output_scanline_cb_t ScanlineCallback;

  // Core 0. Sets clk_sys to 252 MHz, sets up video and audio. Call before
  // setup1() reaches runCore1(). Returns false if cb is null or the clock
  // could not be set.
  bool begin(ScanlineCallback cb, uint32_t sample_rate = 48000);

  // Core 1. Call from setup1(); waits for begin(), then never returns.
  void runCore1(void);

  // Queue interleaved stereo frames (L, R, L, R ...) in groups of 4.
  // Returns frames queued, which may be fewer than requested when the
  // queue is full.
  size_t audioWrite(const int16_t *lr, size_t frames);

  // Frames that can be queued now.
  size_t audioAvailableForWrite(void);

  uint32_t sampleRate(void) const { return _sample_rate; }

private:
  volatile bool _ready = false;
  uint32_t _sample_rate = 48000;
  int _channel_frame = 0;
};

// Adafruit_DVI_Audio with a 320x240 RGB565 GFX canvas (150 KB), shown
// pixel-doubled to 640x480.
class Adafruit_DVI_Audio_GFX16 : public Adafruit_DVI_Audio,
                                 public GFXcanvas16 {
public:
  Adafruit_DVI_Audio_GFX16(void) : GFXcanvas16(320, 240) {}

  // Returns false if the canvas could not be allocated or begin() failed.
  bool begin(uint32_t sample_rate = 48000);
};
