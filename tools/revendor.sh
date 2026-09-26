#!/bin/sh
# Copy a pinned fliperama86/pico_hdmi commit into src/pico_hdmi, unmodified.
# Upstream .c files are renamed .c.inc so Arduino does not compile them
# directly; src/dvi_*.c include them after dvi_audio_config.h.
# video_output_rt.c is left out: it defines the same symbols as video_output.c.
set -e
REV=${1:-a18d800e4db88591296e5830bd834a0b9bc075da}   # tag v0.0.19
ROOT=$(cd "$(dirname "$0")/.." && pwd)
DST=$ROOT/src/pico_hdmi
TMP=$(mktemp -d)
git clone -q https://github.com/fliperama86/pico_hdmi "$TMP/pico_hdmi"
git -C "$TMP/pico_hdmi" checkout -q "$REV"
rm -rf "$DST"
mkdir -p "$DST"
cp "$TMP"/pico_hdmi/include/pico_hdmi/*.h "$DST/"
for f in video_output hstx_data_island_queue hstx_packet hstx_pins; do
  cp "$TMP/pico_hdmi/src/$f.c" "$DST/$f.c.inc"
done
cp "$TMP/pico_hdmi/src/hstx_pins_internal.h" "$DST/"
cp "$TMP/pico_hdmi/LICENSE" "$DST/LICENSE"
echo "$REV" > "$DST/REVISION"
rm -rf "$TMP"
echo "vendored pico_hdmi $REV"
