#!/usr/bin/env python3
"""Check layout using the real pinned LVGL and firmware UI implementation.

This replaces the stale hand-maintained geometry estimator. The host renderer
checks all 20 views, every celebration message, 500 rapid page transitions,
memory stability, idle animation completion and screen recreation.
"""
from pathlib import Path
import subprocess
from simulate import executable

ROOT = Path(__file__).resolve().parents[1]

if __name__ == "__main__":
    output = ROOT / "build/preview-v3"
    output.mkdir(parents=True, exist_ok=True)
    raise SystemExit(subprocess.run([str(executable()), str(output)]).returncode)
