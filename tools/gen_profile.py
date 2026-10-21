#!/usr/bin/env python3
"""Generate main/profile_data.h from data/profile.json.

The device does not parse JSON at runtime. This turns the single editable
data source into C string constants so the firmware can be rebuilt after
editing data/profile.json alone.

Each entry item has three fixed lines:
    period  - year / tag shown in the accent column, e.g. "2013"
    line1   - the role or degree, e.g. "PhD, HCI & 3D Modelling"
    line2   - the organisation, e.g. "University of Waikato, NZ"

Those three lines are pre-wrapped here (not in the UI) so the firmware only
ever assigns short strings to fixed-height labels. tools/check_layout.py
verifies that the pre-wrapped result actually fits the 240x320 screen.

Usage:
    python tools/gen_profile.py
"""

import json
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
SRC = os.path.join(ROOT, "data", "profile.json")
OUT = os.path.join(ROOT, "main", "profile_data.h")

# Must match the label geometry in main/passport_ui.c.
# 240px is narrow, so each page shows only 2 entries. Each entry is a
# full-width card: the period tag sits on its own line above the
# description, so the description gets the entire card width.
CONTENT_W = 184
CARD_PAD = 12
TEXT_W = CONTENT_W - CARD_PAD * 2
ENTRY_FONT = 14
LINE_H = 19
# Cards are a fixed height, so the line count is fixed too. Two cards must fit
# the body area: (17 + 3*19 + 8*2)*2 + 8 = 188px inside a 190px body.
MAX_LINES_PER_ENTRY = 3
ITEMS_PER_PAGE = 2
CARD_PAD_Y = 8
CARD_GAP = 8
PERIOD_LINE_H = 17

EMAIL_FONT = 14
# CONTACT 页:二维码用纠错等级 Q(37 模块,含静区 45 单位)。
# 每模块 3px -> 135px,给下方文字留出 45px。tools/verify_qr.py 已验证
# 3px/模块在轻微模糊(屏幕反光与摩尔纹)下仍能正常解码,而 4px(180px)
# 会一直顶到页脚,导致下方一行文字都放不下。
QR_QUIET = 4
QR_SCALE = 3
QR_HEADER = os.path.join(ROOT, "main", "qr_profile.h")


def qr_module_count():
    """Read the real module count from the generated QR header."""
    if not os.path.isfile(QR_HEADER):
        sys.exit("missing main/qr_profile.h; run python tools/gen_qr.py")
    with open(QR_HEADER, "r", encoding="utf-8") as fh:
        for line in fh:
            if line.startswith("#define QR_MODULE_COUNT"):
                return int(line.split()[-1])
    sys.exit("QR_MODULE_COUNT not found in main/qr_profile.h")


QR_SIDE = (qr_module_count() + QR_QUIET * 2) * QR_SCALE
# CONTACT email occupies its own full-width block below the QR.
TEXT_EMAIL_W = CONTENT_W


def c_string(value):
    escaped = (
        value.replace("\\", "\\\\")
        .replace('"', '\\"')
        .replace("\n", "\\n")
        .replace("\r", "")
    )
    return f'"{escaped}"'


def measure(metrics, size, text):
    table = metrics.get(str(size), {})
    return sum(table.get(ch, size * 0.55) for ch in text)


def wrap(metrics, size, text, max_w):
    """Wrap on spaces; never split a word, so a too-long word is reported."""
    lines, current = [], ""
    for word in text.split():
        candidate = f"{current} {word}".strip()
        if measure(metrics, size, candidate) <= max_w or not current:
            current = candidate
        else:
            lines.append(current)
            current = word
    if current:
        lines.append(current)
    return lines


def load_metrics():
    import subprocess
    out = subprocess.run(
        [sys.executable, os.path.join(HERE, "measure_font.py")],
        capture_output=True, text=True, encoding="utf-8", check=True,
    )
    return json.loads(out.stdout)


def flatten(metrics, item):
    """Return up to MAX_LINES_PER_ENTRY display lines for one entry."""
    raw = [item["line1"], item.get("line2", "")]
    lines = []
    for part in raw:
        if not part:
            continue
        wrapped = wrap(metrics, ENTRY_FONT, part, TEXT_W)
        lines.extend(wrapped)
    if len(lines) > MAX_LINES_PER_ENTRY:
        raise SystemExit(
            f"条目内容超过 {MAX_LINES_PER_ENTRY} 行（每行 {TEXT_W:.0f}px / {ENTRY_FONT}px 字号），"
            f"请精简 line1/line2:\n  {item}\n  实际需要 {len(lines)} 行"
        )
    while len(lines) < MAX_LINES_PER_ENTRY:
        lines.append("")
    return lines[:MAX_LINES_PER_ENTRY]


def main():
    if not os.path.isfile(SRC):
        sys.exit(f"missing data source: {SRC}")

    with open(SRC, "r", encoding="utf-8") as handle:
        data = json.load(handle)
    metrics = load_metrics()
    if QR_SIDE > 135:
        sys.exit("The URL needs a larger QR than the contact layout supports (135px). Shorten profileUrl or redesign the contact layout before building.")
    checks = ((16, data["name"], CONTENT_W, 2, "name"),
              (14, data["title"], CONTENT_W, 1, "title"),
              (14, data["institution"], CONTENT_W, 2, "institution"))
    for size, value, width, maximum, field in checks:
        if len(wrap(metrics, size, value, width)) > maximum:
            sys.exit(f"{field} needs more than {maximum} display lines; shorten it or adjust passport_ui.c")
    for tag in data["tags"]:
        if measure(metrics, 14, tag) > 43:
            sys.exit(f"Tag {tag!r} is too wide for its 43px badge")

    out = []
    out.append("// profile_data.h - GENERATED FILE, DO NOT EDIT BY HAND.")
    out.append("// Source: data/profile.json  |  Regenerate: python tools/gen_profile.py")
    out.append("// Edit data/profile.json instead, then rebuild the firmware.")
    out.append("// Entry text is pre-wrapped to fit a " + str(TEXT_W) + "px column.")
    out.append("#pragma once")
    out.append("")
    out.append("// Geometry shared with main/passport_ui.c and tools/check_layout.py.")
    out.append(f"#define AZMI_CONTENT_W     {CONTENT_W}")
    out.append(f"#define AZMI_ENTRY_LINES   {MAX_LINES_PER_ENTRY}")
    out.append(f"#define AZMI_ITEMS_PER_PAGE {ITEMS_PER_PAGE}")
    out.append(f"#define AZMI_LINE_H        {LINE_H}")
    out.append(f"#define AZMI_QR_SIDE       {QR_SIDE}")
    out.append(f"#define AZMI_QR_SCALE     {QR_SCALE}")
    out.append(f"#define AZMI_QR_QUIET     {QR_QUIET}")
    out.append(f"#define AZMI_EMAIL_LINE_W {TEXT_EMAIL_W}")
    out.append(f"#define AZMI_CARD_PAD_Y   {CARD_PAD_Y}")
    out.append(f"#define AZMI_CARD_GAP     {CARD_GAP}")
    out.append(f"#define AZMI_PERIOD_H     {PERIOD_LINE_H}")
    out.append("")
    out.append(f'#define AZMI_NAME        {c_string(data["name"])}')
    out.append(f'#define AZMI_NAME_DISPLAY {c_string(chr(10).join(wrap(metrics, 16, data["name"], CONTENT_W)))}')
    out.append(f'#define AZMI_TITLE       {c_string(data["title"])}')
    out.append(f'#define AZMI_INSTITUTION {c_string(data["institution"])}')
    # The institution needs 194 px at 14 px, more than the 184 px content
    # width, so it is pre-split into two centred lines.
    inst_lines = wrap(metrics, 14, data["institution"], CONTENT_W)
    out.append(f'#define AZMI_INST_LINE1  {c_string(inst_lines[0])}')
    out.append(f'#define AZMI_INST_LINE2  '
               f'{c_string(inst_lines[1] if len(inst_lines) > 1 else "")}')
    out.append(f'#define AZMI_EMAIL       {c_string(data["email"])}')
    out.append(f'#define AZMI_OFFICE      {c_string(data["office"])}')
    out.append(f'#define AZMI_PROFILE_URL {c_string(data["profileUrl"])}')
    out.append("")

    # Split the email into two full-width lines at a punctuation boundary.
    email = data["email"]
    width = lambda s: measure(metrics, 14, s)
    candidates = [i for i, ch in enumerate(email) if ch in "@."]
    cut = next(
        (i for i in candidates
         if 0 < i < len(email) - 1
         and width(email[:i]) <= TEXT_EMAIL_W
         and width(email[i:]) <= TEXT_EMAIL_W),
        None,
    )
    if cut is None:
        sys.exit(
            f"邮箱 {email!r} 无法在 {TEXT_EMAIL_W:.0f}px 列宽内断成两行。"
            f"请缩短邮箱,或重新设计联系页的邮箱区域。"
        )
    out.append(f'#define AZMI_EMAIL_L1   {c_string(email[:cut])}')
    out.append(f'#define AZMI_EMAIL_L2   {c_string(email[cut:])}')
    out.append("")
    out.append("#define AZMI_TAG_COUNT %d" % len(data["tags"]))
    out.append("static const char *const AZMI_TAGS[AZMI_TAG_COUNT] = {")
    for tag in data["tags"]:
        out.append(f"    {c_string(tag)},")
    out.append("};")
    out.append("")

    # Page groups. The order here defines the AZMI_*_PAGE_* names the UI uses,
    # so keep it in sync with PASSPORT_PAGE_* in main/passport_nav.h.
    GROUPS = (
        ("JOURNEY", "journeyPages"),
        ("RESEARCH", "researchPages"),
        ("AWARDS", "awardsPages"),
        ("IMPACT", "impactPages"),
        ("TEACHING", "teachingPages"),
    )

    total_subpages = 0
    for group, key in GROUPS:
        pages = data.get(key) or []
        if not pages:
            raise SystemExit(f"data/profile.json 缺少 {key}")
        total_subpages += len(pages)
        prefix = f"AZMI_{group}"
        out.append(f"#define {prefix}_PAGE_COUNT {len(pages)}")
        out.append(f"static const char *const {prefix}_PAGE_TITLES[{prefix}_PAGE_COUNT] = {{")
        for page in pages:
            out.append(f"    {c_string(page['title'])},")
        out.append("};")
        out.append(f"static const char *const {prefix}_PERIODS"
                   f"[{prefix}_PAGE_COUNT][{ITEMS_PER_PAGE}] = {{")
        for page in pages:
            periods = [c_string(i["period"]) for i in page["items"][:ITEMS_PER_PAGE]]
            while len(periods) < ITEMS_PER_PAGE:
                periods.append('""')
            out.append("    { " + ", ".join(periods) + " },")
        out.append("};")
        out.append(f"static const char *const {prefix}_LINES"
                   f"[{prefix}_PAGE_COUNT][{ITEMS_PER_PAGE}][{MAX_LINES_PER_ENTRY}] = {{")
        for page in pages:
            block = []
            for item in page["items"][:ITEMS_PER_PAGE]:
                block.append("{ " + ", ".join(c_string(x) for x in flatten(metrics, item)) + " }")
            while len(block) < ITEMS_PER_PAGE:
                block.append("{ " + ", ".join(['""'] * MAX_LINES_PER_ENTRY) + " }")
            out.append("    { " + ", ".join(block) + " },")
        out.append("};")
        out.append("")

    # Total number of OK-cyclable subpages across all list pages; the UI uses it
    # to decide where the footer shows a subpage counter.
    out.append(f"#define AZMI_LIST_SUBPAGES {total_subpages}")
    out.append("")

    with open(OUT, "w", encoding="utf-8", newline="\n") as handle:
        handle.write("\n".join(out))
    print(f"generated {os.path.relpath(OUT, ROOT)}")


if __name__ == "__main__":
    main()
