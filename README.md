[简体中文](README.zh_CN.md) · **English**

# Gift for Azmi

A small, cheerful pocket passport for Dr. Azmi, made as a thank-you gift from a
student. It celebrates a teacher's journey, ideas and influence through a warm
cartoon portrait, colorful academic cards, and eight little animated stories.

**Dear Dr. Azmi: this is yours to explore, take apart and make your own.**
The editable profile, artwork, animation code, generators and tests are included.
No cloud account, AI API key, Wi-Fi connection or subscription is needed to run
the device or rebuild the included artwork.

![The eight chapters](assets/screenshots/overview.png)

<details>
<summary>See the chapter animations</summary>

![Animated chapter stickers](assets/screenshots/sections.gif)

These are actual LVGL host renders, not photographs of the device. The preview
loops; on the device each chapter's sticker plays for about 3.2 seconds and then
rests. The preview battery reading is sample data.

</details>

## What is inside

Portrait, profile, journey, research, awards, impact, teaching and contact: eight
chapters with 20 views in the included profile. Each chapter has its own color
and animated illustration: a smiling sun, explorer card, winding route, VR
goggles, gold cup, heart ripples, a book with an idea, and an opening envelope.
The contact QR links to the included university profile URL.

| Button | Action |
| --- | --- |
| UP / DOWN | Previous / next chapter; wraps around |
| OK on a chapter with several views | Next view within that chapter |
| OK on the portrait | A short celebration and another thank-you message |

The app is offline and read-only at runtime. DIY edits happen on your computer,
followed by a rebuild and flash. It does not record audio, connect to a server,
or collect usage data.

## Hardware

FoloToy AI Passport: **ESP32-C3, 8 MB Flash, no PSRAM, 240 × 320 color display,
three buttons**, optional CW2017 battery gauge. Firmware uses **ESP-IDF 5.5.3**
and the dependencies pinned in [dependencies.lock](dependencies.lock), including
LVGL 9.5.0. The default partition layout is NVS, PHY data and one factory app.
Other ESP32 boards need a compatible BSP; do not assume their wiring matches.

## Get started

Clone or download this repository and open an activated ESP-IDF 5.5.3 terminal
in its root folder. The generated C headers are committed, so an unchanged
firmware build does not need Pillow or a network image service.

```bash
git clone YOUR_REPOSITORY_URL gift-for-azmi
cd gift-for-azmi
idf.py --version
```

Windows PowerShell, in the ESP-IDF terminal:

```powershell
.\tools\build.ps1
```

Linux / macOS, after sourcing ESP-IDF's `export.sh`:

```bash
./tools/validate.sh --firmware
```

Both paths build from `sdkconfig.defaults` without overwriting a local root
`sdkconfig`, verify the merged image, and retain matching ELF/MAP debugging
artifacts. The flashable output is `build/FoloToy-AI-Passport-full.bin`.
See the [environment guide](docs/development/engineering/environment-setup.md)
for ESP-IDF installation and activation.

## Make it your own

Install the small host-side asset tools in a Python 3.10+ environment:

```bash
python -m pip install -r requirements.txt
```

| I want to change... | Edit this |
| --- | --- |
| Name, title, university, office, email, QR destination | [data/profile.json](data/profile.json) |
| Academic entries and number of views | The five `*Pages` arrays in the same JSON file |
| Portrait | A square PNG/JPEG of your choice; keep a reusable copy under `assets/images/` |
| Chapter headings and colors | `THEMES` in [main/passport_ui.c](main/passport_ui.c) |
| Thank-you messages | `messages` in `passport_ui_celebrate()` in the same file |
| Chapter illustrations and frame movements | [main/passport_art.c](main/passport_art.c) |
| Button behavior and page order | [main/main.c](main/main.c), [main/passport_nav.h](main/passport_nav.h) and [main/passport_nav.c](main/passport_nav.c) |
| Board wiring and reusable hardware behavior | [components/bsp](components/bsp), starting with `include/bsp_pins.h` |

After editing the JSON, regenerate all embedded resources:

```bash
python tools/regenerate.py
```

To use your own portrait:

```bash
python tools/regenerate.py --avatar assets/images/my-portrait.png
```

The portrait becomes a 144 × 144 full-color RGB565 image in Flash. The script
does not cartoonize a new photograph: provide your own finished illustration
or photo. The included cartoon can be rebuilt without image generation.
The original reference photograph and discarded alternatives are not included.

Each content page takes **one or two entries**. Keep entry descriptions short:
at most three wrapped lines in a 160 px column. Add another page to a `*Pages`
array for more content; existing chapter navigation picks up its count.
Use up to four short research tags. An overly long URL may need a larger QR
than the current contact layout supports; the generator reports that rather
than silently clipping it. Fix generation errors before building.

The current fonts support printable ASCII text. For Chinese or other scripts,
follow the [font guide](docs/development/engineering/lvgl-chinese-fonts.md),
add the needed glyphs and check the actual display. UTF-8 alone is not a font.
The sample biography is an editable gift draft, not an authoritative CV; please
review and correct it before using it professionally.

## Preview and test

After the first IDF configuration/build has downloaded Managed Components,
build the actual LVGL host renderer with CMake and a desktop C compiler:

```bash
cmake -S tests/ui_preview -B build/ui-host
cmake --build build/ui-host
python tools/simulate.py
```

On Windows with MinGW and Ninja, select them explicitly if CMake cannot find a
compiler: add `-G Ninja -DCMAKE_C_COMPILER=gcc` to the configure command.
The renderer checks labels, every chapter's changing illustration frames,
500 rapid transitions, animation cleanup and memory stability. It exports
`build/preview-v3/overview.png` and `build/preview-v3/sections.gif`.
Rebuild the host renderer after regenerating C headers.

The repository and host test gate on Linux/macOS is:

```bash
./tools/validate.sh --static
./tools/validate.sh
```

The complete gate also requires activated ESP-IDF. For optional software QR
decoding, install `requirements-preview.txt` and run `python tools/verify_qr.py`.
GitHub Actions runs static tests and a verified firmware build on pushes to
`main`. The inherited upstream-sync job is disabled so it cannot replace the
gift app with the original hardware-test demo.

## Flash your device

Use a USB data cable and find the actual serial port. **COM8 was the original
development computer's port, not a fixed project setting.** Close other serial
monitors before flashing:

```bash
python -m serial.tools.list_ports
python -m esptool --chip esp32c3 -p YOUR_PORT -b 460800 write_flash 0x0 build/FoloToy-AI-Passport-full.bin
```

The merged `full.bin` belongs at **0x0**. The app-only `.bin` does not.
Merged flashing may reset NVS/settings. Do not erase the entire chip as a
routine step. See [firmware layout and data preservation](docs/development/engineering/firmware-layout.md).
After flashing, check portrait colors, every chapter, UP/DOWN, OK celebrations,
animation smoothness, and QR scanning on your phone. Successful flashing alone
does not confirm the physical screen or buttons.

## Project map

```text
data/                 editable profile and cached font-width metrics
assets/images/        final reusable portrait and its creation prompt
assets/screenshots/   README previews
main/passport_*.c     application UI, navigation and chapter animations
components/bsp/       board drivers and public hardware APIs
tools/                resource generation, preview, validation and packaging
tests/                host tests and the real-LVGL preview harness
docs/                 inherited hardware and engineering reference
skills/               optional workflows for AI-assisted development
```

`main/demo_*.c` and their tests are retained as hardware examples; they are not
compiled into this app. Build output, local toolchains, raw serial logs,
credentials, Python caches and unused experiments are excluded from the repo.

## License and thanks

Code is released under the [MIT License](LICENSE). This is a derivative of
[FoloToy/ai-passport](https://github.com/FoloToy/ai-passport); its original
copyright notice and engineering references are preserved. ESP-IDF and Managed
Components retain their own licenses.

The cartoon was AI-generated from a reference photo supplied during this gift's
creation. The prompt and final illustration are included; the source photo is
not. The code license does not grant rights to someone else's name, likeness or
university branding. Use your own portrait and profile for another person's
passport. Academic source: [Dr. Azmi's university profile](https://www.xmu.edu.my/staff/dr-azmi-bin-mohd-yusof).

Local verification on 2026-10-21: firmware build, static/host checks, merged-image
and ELF identity checks, USB flashing and startup logs passed. Physical colors,
frame rate, button feel and phone scanning still need the owner's observation.
