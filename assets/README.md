<p align="right">
  <a href="README.zh_CN.md">简体中文</a> · <strong>English</strong>
</p>

# Assets

This directory stores reusable fonts, images, music, and sound effects, organized by asset type.

Keep each asset in the matching subdirectory and document its destination, naming, integration method, and source/license. Do not mix binary assets with Markdown documentation.

## Fonts

Store reusable font files and generated font sources in `fonts/`.

- Use descriptive names that include the family, weight, size, and format when relevant.
- Document the source, license, character range, conversion command, and expected destination.
- Check Flash and internal-RAM impact before adding a font; the ESP32-C3 has no PSRAM.
- Do not commit fonts whose license does not permit redistribution.

## Images

Store reusable source images and generated display assets in `images/`.

| File | Dimensions and format | Use and source |
| --- | --- | --- |
| [`images/azmi-cartoon-v2.png`](images/azmi-cartoon-v2.png) | Square PNG; device version 144 × 144 RGB565 | Final cheerful cartoon portrait. Generated with built-in imagegen on 2026-10-21 from a reference supplied for the gift; preserves identity, natural colors and a gentle smile. Run `python tools/gen_avatar.py` to produce `main/avatar_data.h` (41,472 Flash bytes, no mutable portrait buffer). The original reference photo and discarded variants are not distributed. |
| [`images/azmi-cartoon-prompt.txt`](images/azmi-cartoon-prompt.txt) | UTF-8 text | Final generation prompt and source reference. Illustration creation uses imagegen; the Python asset tool only resizes and packs RGB565. |
| [`screenshots/overview.png`](screenshots/overview.png) and [`screenshots/sections.gif`](screenshots/sections.gif) | 1080 × 764, PNG / GIF | Actual host renders for the root README, exported by `tools/simulate.py`. Battery 87% is sample data, not a device measurement. |
| [`images/home.jpg`](images/home.jpg) | 3840 × 2160, JPEG | Product hero image embedded in both project README files to foreground AI Passport and its open, maker-oriented identity. |
| [`images/readme-hardware-specs.png`](images/readme-hardware-specs.png) | 2172 × 724, PNG RGBA | Optional technical infographic retained as a reference asset; it is no longer used as the homepage hero. Generated for this repository with the built-in image generation tool on 2026-09-17; the six labels and values were checked against the documented hardware contract. |
| [`images/logo-wordmark.png`](images/logo-wordmark.png) | 1648 × 336, PNG RGBA | Transparent black wordmark extracted from the repository's original `images/logo.png`; embedded in both project README files for light backgrounds. |
| [`images/logo-wordmark-dark.png`](images/logo-wordmark-dark.png) | 1648 × 336, PNG RGBA | White version of the extracted wordmark, used by the README `<picture>` element when GitHub is in dark mode. |

- Use descriptive names and document dimensions, pixel format, conversion steps, and destination.
- Prefer formats suitable for the 240 × 320 RGB565 display and account for Flash and internal RAM.
- Preserve editable sources where licensing permits, and record the source and license.
- Never commit device QR secrets, credentials, or personal data in images.

### Gift UI preview

Build the actual LVGL renderer with `cmake -S tests/ui_preview -B build/ui-host`
and `cmake --build build/ui-host`. The pinned LVGL Managed Component must already
be available. Run `python tools/simulate.py` to check all 20 views, four
celebration messages, 500 rapid transitions and animation cleanup, then export
PNG captures, `build/preview-v3/overview.png` and `build/preview-v3/motion.gif`.
These are host renders of `main/passport_ui.c`, not device screenshots. The
87% battery value in previews is sample data. Real firmware uses the BSP gauge.

Each of the eight chapters also has a 40 × 42 native illustrated sticker in
`main/passport_art.c`: smiling sun, explorer card, travelling route, VR goggles,
gold cup, heart ripples, book with an idea, and an opening envelope. Each uses an
eight-frame movement cycle, plays briefly on entry, and settles after 3.2 seconds.
The stickers draw directly with LVGL shapes, need no raster asset or extra image
buffer, and stay in a reserved header slot outside the body and QR code. The
preview checks at least three distinct rendered frames per chapter and exports
the eight animated pages together as `build/preview-v3/sections.gif`.

## Music and sound effects

Store reusable music and sound-effect sources in `music/`.

- Document the source, license, sample rate, bit depth, channels, conversion command, and destination.
- Prefer 16 kHz, 16-bit mono PCM when it matches the current BSP audio path.
- Check Flash and internal-RAM cost before embedding audio; stream or chunk long recordings.
- Do not commit media without redistribution permission.
