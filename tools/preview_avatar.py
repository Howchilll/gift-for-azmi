#!/usr/bin/env python3
"""Decode the firmware's actual RGB565 portrait, for a faithful asset preview."""
from pathlib import Path
import re
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]

if __name__ == "__main__":
    header = (ROOT / "main/avatar_data.h").read_text(encoding="utf-8")
    side = int(re.search(r"#define AVATAR_SIDE (\d+)", header).group(1))
    array = header.split("AZMI_AVATAR_PIXELS[] = {", 1)[1].split("};", 1)[0]
    data = bytes(int(v, 16) for v in re.findall(r"0x([0-9a-f]{2})", array))
    pixels = []
    for i in range(0, len(data), 2):
        p = int.from_bytes(data[i:i + 2], "little")
        pixels.append((((p >> 11) & 31) * 255 // 31,
                       ((p >> 5) & 63) * 255 // 63, (p & 31) * 255 // 31))
    image = Image.new("RGB", (side, side))
    image.putdata(pixels)
    output = ROOT / "build/preview-v2/avatar-rgb565.png"
    output.parent.mkdir(parents=True, exist_ok=True)
    image.save(output)
    print(output)
