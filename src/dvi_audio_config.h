// SPDX-FileCopyrightText: 2026 Adafruit Industries
// SPDX-License-Identifier: MIT
//
// Build flags for the vendored pico_hdmi sources. Arduino does not run
// pico_hdmi's CMake, so its option() defaults are restated here.
#pragma once

// CMake default ON: exact rational audio pacing, no drift against ACR.
#define PICO_HDMI_EXACT_AUDIO_PACING 1
// CMake defaults OFF.
#define PICO_HDMI_LINE_BUFFER_IN_SCRATCH_Y 0
#define PICO_HDMI_PRECOMPOSED_ACTIVE_LINES 0
#define PICO_HDMI_FRANK_PAD_CONFIG 0
#define PICO_HDMI_PIXEL_FORMAT_RGB888 0
#define PICO_HDMI_EXPLICIT_AUDIO_CHANNEL_STATUS 0

// clk_sys runs at 252 MHz (a multiple of 12 MHz, so PIO USB host still
// works). HSTX = 252 / 2 = 126 MHz, pixel clock 126 / 5 = 25.2 MHz.
#define DVI_AUDIO_SYS_KHZ 252000
#define MODE_HSTX_CLK_DIV 2
