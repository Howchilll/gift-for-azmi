#!/usr/bin/env python3
"""Generate main/qr_profile.h from data/profile.json.

The QR code is encoded offline and stored one tone per byte, so the firmware
needs no QR library at runtime.

Two choices matter for a 240x320 screen:

* Error correction Q (25%), not M. The code is only ~180 px wide on a small
  glossy panel where reflections and moire hurt contrast; Q tolerates roughly
  a quarter of the symbol being damaged.
* No quiet zone is baked in. The UI adds it while drawing, so the modules
  themselves get maximum size within the content width.

Usage:
    pip install "qrcode[pil]"
    python tools/gen_qr.py
"""

import json
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
SRC = os.path.join(ROOT, "data", "profile.json")
OUT = os.path.join(ROOT, "main", "qr_profile.h")

# The UI draws the quiet zone; keep these in sync with main/passport_ui.c.
QUIET_MODULES = 4


def main():
    try:
        import qrcode
    except ImportError:
        sys.exit('missing dependency: pip install "qrcode[pil]"')

    if not os.path.isfile(SRC):
        sys.exit(f"missing data source: {SRC}")
    with open(SRC, "r", encoding="utf-8") as handle:
        url = json.load(handle)["profileUrl"]

    qr = qrcode.QRCode(
        error_correction=qrcode.constants.ERROR_CORRECT_Q,
        box_size=1,
        border=0,
    )
    qr.add_data(url)
    qr.make(fit=True)
    matrix = qr.get_matrix()

    size = len(matrix)
    out = [
        "// qr_profile.h - GENERATED FILE, DO NOT EDIT BY HAND.",
        "// Source: data/profile.json (profileUrl)  |  Regenerate: python tools/gen_qr.py",
        f"// Payload: {url}",
        f"// QR version {qr.version}, error correction level Q, {size}x{size} modules.",
        f"// The UI adds a {QUIET_MODULES}-module quiet zone when drawing.",
        "#pragma once",
        "",
        "#include <stdint.h>",
        "",
        f"#define QR_MODULE_COUNT {size}",
        f"#define QR_QUIET_MODULES {QUIET_MODULES}",
        f'#define QR_PAYLOAD_URL "{url}"',
        "",
        "// Row-major, one byte per module, 1 = dark.",
        f"static const uint8_t QR_MODULES[QR_MODULE_COUNT][QR_MODULE_COUNT] = {{",
    ]
    for row in matrix:
        out.append("    { " + ", ".join("1" if cell else "0" for cell in row) + " },")
    out.append("};")
    out.append("")

    with open(OUT, "w", encoding="utf-8", newline="\n") as handle:
        handle.write("\n".join(out))

    total = size + QUIET_MODULES * 2
    print(f"generated {os.path.relpath(OUT, ROOT)} "
          f"(version {qr.version}, {size}x{size}, {total} units with quiet zone)")


if __name__ == "__main__":
    main()