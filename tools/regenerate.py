#!/usr/bin/env python3
"""Validate editable content, then rebuild the QR, profile and portrait headers.

Usage: python tools/regenerate.py [--avatar path/to/your-picture.png]
Committed font metrics let this work immediately after cloning, before IDF has
downloaded LVGL. Building/flashing is a separate explicit step.
"""
import argparse
import json
from pathlib import Path
import subprocess
import sys
from urllib.parse import urlsplit

ROOT = Path(__file__).resolve().parents[1]
GROUPS = ("journeyPages", "researchPages", "awardsPages", "impactPages", "teachingPages")


def validate(data):
    def text(value, field):
        if not isinstance(value, str) or not value.strip():
            raise ValueError(f"{field}: enter a nonempty string")
        if not value.isascii() or any(not 32 <= ord(c) <= 126 for c in value):
            raise ValueError(f"{field}: this font supports printable ASCII; see docs/development/engineering/lvgl-chinese-fonts.md for other scripts")
    for key in ("name", "title", "institution", "email", "office", "profileUrl"):
        text(data.get(key), key)
    if "@" not in data["email"]:
        raise ValueError("email: include an @ sign")
    url = urlsplit(data["profileUrl"])
    if url.scheme not in ("https", "http") or not url.netloc:
        raise ValueError("profileUrl: enter a complete http(s) URL")
    tags = data.get("tags")
    if not isinstance(tags, list) or not 1 <= len(tags) <= 4:
        raise ValueError("tags: use one to four short labels")
    for index, tag in enumerate(tags):
        text(tag, f"tags[{index}]")
    for group in GROUPS:
        pages = data.get(group)
        if not isinstance(pages, list) or not pages:
            raise ValueError(f"{group}: include at least one page")
        for index, page in enumerate(pages):
            text(page.get("title"), f"{group}[{index}].title")
            items = page.get("items")
            if not isinstance(items, list) or not 1 <= len(items) <= 2:
                raise ValueError(f"{group}[{index}]: use one or two items per page; add another page for more")
            for item in items:
                for key in ("period", "line1"):
                    text(item.get(key), f"{group}.{key}")
                if item.get("line2"):
                    text(item["line2"], f"{group}.line2")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--avatar", type=Path, help="Your square PNG/JPEG; defaults to the included cartoon")
    args = parser.parse_args()
    try:
        validate(json.loads((ROOT / "data/profile.json").read_text(encoding="utf-8")))
    except (ValueError, TypeError, AttributeError) as error:
        parser.error(str(error))
    if args.avatar and not args.avatar.is_file():
        parser.error(f"Avatar does not exist: {args.avatar}")
    for name in ("gen_qr.py", "gen_profile.py", "gen_avatar.py", "gen_startup_sound.py"):
        command = [sys.executable, str(ROOT / "tools" / name)]
        if name == "gen_avatar.py" and args.avatar:
            command.append(str(args.avatar.resolve()))
        subprocess.run(command, cwd=ROOT, check=True)
    print("Assets regenerated. Build and preview the firmware before flashing.")


if __name__ == "__main__":
    main()
