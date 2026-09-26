// SPDX-FileCopyrightText: 2026 Adafruit Industries
// SPDX-License-Identifier: MIT
#include "Adafruit_DVI_Audio.h"

#include "hardware/clocks.h"
#include "hardware/vreg.h"

// Queue depth pico_hdmi's own example keeps topped up.
#define DVI_AUDIO_QUEUE_TARGET 200
#define DVI_AUDIO_FRAMES_PER_PACKET 4

// Canvas being scanned out; read on core 1 from the scanline IRQ.
static const uint16_t *canvas_pixels;

// Doubles one 320-pixel canvas row into a 640-pixel output line, and each
// canvas row into two output lines.
static void __not_in_flash_func(canvas_scanline)(uint32_t v_scanline,
                                                 uint32_t active_line,
                                                 uint32_t *dst) {
  (void)v_scanline;
  const uint16_t *src = canvas_pixels + (active_line >> 1) * 320;
  for (int i = 0; i < 320; i++) {
    uint32_t p = src[i];
    dst[i] = p | (p << 16);
  }
}

bool Adafruit_DVI_Audio_GFX16::begin(uint32_t sample_rate) {
  canvas_pixels = getBuffer();
  if (!canvas_pixels) {
    return false;
  }
  return Adafruit_DVI_Audio::begin(canvas_scanline, sample_rate);
}

bool Adafruit_DVI_Audio::begin(ScanlineCallback cb, uint32_t sample_rate) {
  if (!cb) {
    return false;
  }
  _sample_rate = sample_rate;
  vreg_set_voltage(VREG_VOLTAGE_1_15);
  delay(10);
  if (!set_sys_clock_khz(DVI_AUDIO_SYS_KHZ, false)) {
    return false;
  }
  hstx_di_queue_init();
  video_output_init(MODE_H_ACTIVE_PIXELS, MODE_V_ACTIVE_LINES);
  pico_hdmi_set_audio_sample_rate(sample_rate);
  video_output_set_scanline_callback(cb);
  _ready = true;
  return true;
}

void Adafruit_DVI_Audio::runCore1(void) {
  while (!_ready) {
    tight_loop_contents();
  }
  video_output_core1_run();
}

size_t Adafruit_DVI_Audio::audioAvailableForWrite(void) {
  uint32_t level = hstx_di_queue_get_level();
  if (level >= DVI_AUDIO_QUEUE_TARGET) {
    return 0;
  }
  return (DVI_AUDIO_QUEUE_TARGET - level) * DVI_AUDIO_FRAMES_PER_PACKET;
}

size_t Adafruit_DVI_Audio::audioWrite(const int16_t *lr, size_t frames) {
  size_t done = 0;
  while (frames - done >= DVI_AUDIO_FRAMES_PER_PACKET) {
    audio_sample_t samples[DVI_AUDIO_FRAMES_PER_PACKET];
    for (int i = 0; i < DVI_AUDIO_FRAMES_PER_PACKET; i++) {
      samples[i].left = lr[(done + i) * 2];
      samples[i].right = lr[(done + i) * 2 + 1];
    }
    hstx_packet_t packet;
    int next = hstx_packet_set_audio_samples_cs_rate(
        &packet, samples, DVI_AUDIO_FRAMES_PER_PACKET, _channel_frame,
        _sample_rate);
    hstx_data_island_t island;
    hstx_encode_data_island(&island, &packet, false, DI_HSYNC_ACTIVE);
    if (!hstx_di_queue_push(&island)) {
      break;
    }
    _channel_frame = next;
    done += DVI_AUDIO_FRAMES_PER_PACKET;
  }
  return done;
}
