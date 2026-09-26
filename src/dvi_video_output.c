// SPDX-FileCopyrightText: 2026 Adafruit Industries
// SPDX-License-Identifier: MIT
#include "dvi_audio_config.h"

#include <stdbool.h>
#include <stdint.h>

#include "pico.h"

// pico_hdmi calls this from its per-scanline DMA IRQ but leaves it in flash.
// A flash cache miss there can overrun the line and desync HSTX for good
// (frames then "complete" at ~138 Hz). Declaring it here first puts the
// upstream definition in RAM without editing the vendored file.
static uint32_t __not_in_flash_func(build_line_with_di)(uint32_t *buf,
                                                        const uint32_t *di_words,
                                                        bool vsync, bool active);

#include "pico_hdmi/video_output.c.inc"
