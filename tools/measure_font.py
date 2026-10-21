#!/usr/bin/env python3
"""Measure real Montserrat advance widths from the LVGL font sources.

Why this exists: laying out a 240x320 screen by guessing character widths
produces overlapping text. This script reads the actual `adv_w` values that
LVGL will use at runtime, so the simulator and the layout code can compute
real widths.

LVGL's `lv_font_fmt_txt_glyph_dsc_t.adv_w` is stored in 1/16 px units.
The glyph order matches the bitmap order, which the font file annotates
with `/* U+XXXX "c" */` comments.

Output: JSON on stdout:
    {"14": {"A": 9.1, ...}, ...}
"""

import json
import os
import re
import sys
from pathlib import Path

SIZES = [12, 14, 16, 20, 28]


def find_font_dir():
    here = os.path.dirname(os.path.abspath(__file__))
    root = os.path.dirname(here)
    base = os.path.join(root, "managed_components", "lvgl__lvgl", "src", "font")
    if os.path.isfile(os.path.join(base, "lv_font_montserrat_14.c")):
        return base
    return None


def parse_font(path):
    """Return {char: advance_px} for printable ASCII glyphs."""
    with open(path, "r", encoding="utf-8", errors="replace") as handle:
        text = handle.read()

    # 1) Character for each bitmap slot, in declaration order.
    chars = []
    for m in re.finditer(r'/\*\s*U\+([0-9A-Fa-f]{4})\s+"((?:\\.|[^"\\])*)"\s*\*/', text):
        code = int(m.group(1), 16)
        raw = m.group(2)
        if code == 0:  # id 0 is reserved, has no bitmap comment
            continue
        if 32 <= code <= 126:
            chars.append((code, chr(code)))

    # 2) adv_w per glyph slot, in the same order. Entry 0 is the reserved id.
    dsc_match = re.search(
        r"glyph_dsc\[\]\s*=\s*\{(.*?)\n\};", text, re.S
    )
    if not dsc_match:
        sys.exit(f"glyph_dsc not found in {path}")
    # LVGL rounds each glyph's fixed-point advance, not the final line sum.
    # Summing fractions underestimated long strings enough to wrap on-device.
    advs = [(int(a) + 8) // 16 for a in re.findall(r"\.adv_w\s*=\s*(\d+)", dsc_match.group(1))]

    widths = {}
    # advs[0] is the reserved id; chars[i] corresponds to advs[i + 1].
    for i, (code, ch) in enumerate(chars):
        if i + 1 < len(advs):
            widths[ch] = advs[i + 1]
    return widths


def main():
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--export", type=Path, help="Export the pinned font metrics for clean-clone asset generation")
    args = parser.parse_args()
    base = find_font_dir()
    cache = Path(__file__).resolve().parents[1] / "data/font-metrics.json"
    if base is None:
        if not cache.is_file():
            sys.exit("LVGL fonts and cached data/font-metrics.json are both missing; run idf.py reconfigure first")
        cached = cache.read_text(encoding="utf-8")
        json.loads(cached)
        if args.export:
            sys.exit("Export needs the actual pinned LVGL sources; run idf.py reconfigure first")
        print(cached)
        return
    out = {}
    for size in SIZES:
        path = os.path.join(base, f"lv_font_montserrat_{size}.c")
        if os.path.isfile(path):
            out[str(size)] = parse_font(path)
    out["_source"] = {"lvgl": "9.5.0", "license": "MIT", "rounding": "(adv_w + 8) // 16", "url": "https://github.com/lvgl/lvgl/tree/v9.5.0/src/font"}
    encoded = json.dumps(out, ensure_ascii=False, sort_keys=True, indent=2)
    if args.export:
        args.export.write_text(encoded + "\n", encoding="utf-8")
    else:
        print(encoded)


if __name__ == "__main__":
    main()
