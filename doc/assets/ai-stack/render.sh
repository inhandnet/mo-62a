#!/bin/bash
# SVG → PNG（2x 缩放，保证清晰度）
cd "$(dirname "$0")"
for f in *.svg; do
  rsvg-convert -z 2 -o "${f%.svg}.png" "$f" && echo "  ✓ ${f%.svg}.png"
done
