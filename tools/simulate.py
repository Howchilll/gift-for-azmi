#!/usr/bin/env python3
"""Export actual LVGL captures as PNG, a contact sheet and an animation GIF.

Build tests/ui_preview first (CMake, C compiler, pinned managed LVGL required).
The executable renders the real passport_ui.c and fails on layout/lifetime errors.
Usage: python tools/simulate.py [output-directory] [--captured]
"""
from pathlib import Path
import argparse
import json
import os
import subprocess
from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[1]
NAMES = ["portrait", "profile", "journey", "research", "awards", "impact", "teaching", "contact"]


def executable():
    name = "passport_preview.exe" if os.name == "nt" else "passport_preview"
    result = ROOT / "build/ui-host" / name
    if not result.exists():
        raise SystemExit("Build preview first: cmake -S tests/ui_preview -B build/ui-host; cmake --build build/ui-host")
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("directory", nargs="?", default=str(ROOT / "build/preview-v3"))
    parser.add_argument("--captured", action="store_true", help="Export already-tested PPM captures without rerunning")
    args = parser.parse_args()
    directory = Path(args.directory).resolve()
    directory.mkdir(parents=True, exist_ok=True)
    if not args.captured:
        subprocess.run([str(executable()), str(directory)], check=True)
    data = json.loads((ROOT / "data/profile.json").read_text(encoding="utf-8"))
    counts = [1, 1] + [len(data[key + "Pages"]) for key in NAMES[2:7]] + [1]
    pages = [directory / f"page-{page:02d}-{sub:02d}.ppm"
             for page, count in enumerate(counts, 1) for sub in range(1, count + 1)]
    missing = [str(path) for path in pages if not path.exists()]
    if missing:
        raise SystemExit("Missing current page captures; rebuild the preview: " + ", ".join(missing))
    for path in pages:
        Image.open(path).save(path.with_suffix(".png"))
    sheet = Image.new("RGB", (4 * 264 + 24, 2 * 370 + 24), "#EEE9E0")
    draw = ImageDraw.Draw(sheet)
    for index, name in enumerate(NAMES):
        x, y = 24 + index % 4 * 264, 24 + index // 4 * 370
        page = Image.open(directory / f"page-{index + 1:02d}-01.ppm")
        sheet.paste(page, (x, y))
        draw.text((x + 4, y + 332), f"{index + 1:02d} / {name.upper()}", fill="#263747")
    sheet.save(directory / "overview.png")
    section_frames = []
    for frame in range(8):
        tiled = sheet.copy()
        for index in range(8):
            path = directory / f"section-{index + 1:02d}-frame-{frame:02d}.ppm"
            if not path.exists():
                raise SystemExit(f"Missing themed animation capture: {path}")
            x, y = 24 + index % 4 * 264, 24 + index // 4 * 370
            tiled.paste(Image.open(path), (x, y))
        section_frames.append(tiled)
    section_frames[0].save(directory / "sections.gif", save_all=True,
                           append_images=section_frames[1:], duration=100,
                           loop=0, disposal=2)
    motion = [Image.open(path).convert("RGB") for path in sorted(directory.glob("motion-*.ppm"))]
    if motion:
        motion[0].save(directory / "motion.gif", save_all=True, append_images=motion[1:],
                       duration=40, loop=0, disposal=2)
    print(f"Actual LVGL: {len(pages)} pages, 8 themed animations, {len(motion)} motion frames -> {directory}")


if __name__ == "__main__":
    main()
