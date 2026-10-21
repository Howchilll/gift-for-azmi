"""Verify the generated QR matrix decodes, and at which on-screen sizes.

The device has no way to report a scan result, so the only honest check is
to reproduce the exact pixel grid the firmware draws and run a real decoder
over it. This also answers "how many pixels per module can we get away with".
"""
import os
import sys

import cv2
import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
HEADER = os.path.join(ROOT, "main", "qr_profile.h")


def load():
    size, url, quiet, modules = 0, "", 0, []
    with open(HEADER, encoding="utf-8") as fh:
        for line in fh:
            s = line.strip()
            if s.startswith("#define QR_MODULE_COUNT"):
                size = int(s.split()[-1])
            elif s.startswith("#define QR_QUIET_MODULES"):
                quiet = int(s.split()[-1])
            elif s.startswith("#define QR_PAYLOAD_URL"):
                url = s.split('"')[1]
            elif size and s.startswith("{ ") and len(modules) < size:
                vals = [v.strip() for v in s.strip("{},").split(",")]
                if all(v in ("0", "1") for v in vals) and len(vals) == size:
                    modules.append([int(v) for v in vals])
    if not modules:
        sys.exit("could not parse the QR matrix from qr_profile.h")
    return size, quiet, url, modules


def render(modules, quiet, scale):
    n = len(modules)
    side = (n + quiet * 2) * scale
    img = np.full((side, side), 255, dtype=np.uint8)
    for r in range(n):
        for c in range(n):
            if modules[r][c]:
                y = (r + quiet) * scale
                x = (c + quiet) * scale
                img[y:y + scale, x:x + scale] = 0
    return img


def main():
    size, quiet, url, modules = load()
    print(f"matrix  : {size}x{size}, quiet zone {quiet} modules")
    print(f"payload : {url}")
    print()

    detector = cv2.QRCodeDetector()
    print(f"{'scale':<7}{'side':<8}{'fits 184px':<12}{'decode'}")
    for scale in (2, 3, 4, 5):
        side = (size + quiet * 2) * scale
        img = render(modules, quiet, scale)
        decoded, _, _ = detector.detectAndDecode(img)
        ok = "OK" if decoded == url else "FAIL"
        fits = "yes" if side <= 184 else "no"
        print(f"{scale:<7}{side:<8}{fits:<12}{ok}")

    # Also confirm the matrix survives a blur, which is what a glossy small
    # panel plus camera moire effectively does.
    print()
    print("with a slight blur (simulates screen glare / moire):")
    for scale in (3, 4):
        img = render(modules, quiet, scale)
        blurred = cv2.GaussianBlur(img, (3, 3), 0)
        decoded, _, _ = detector.detectAndDecode(blurred)
        print(f"  scale {scale}: {'OK' if decoded == url else 'FAIL'}")


if __name__ == "__main__":
    main()