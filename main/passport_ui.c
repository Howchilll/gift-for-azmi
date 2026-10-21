// One live page bounds LVGL memory and animation lifetime. Artwork is in Flash.
#include "passport_ui.h"
#include "passport_art.h"
#include "avatar_data.h"
#include "profile_data.h"
#include "qr_profile.h"
#include "lvgl.h"
#include <stdio.h>

#define BG       0xFFF8ED
#define INK      0x263747
#define MUTED    0x596876
#define RULE     0xE3DCD0
#define LEFT     28
#define WIDTH    184
#define BODY_Y   82
#define BODY_H   190
#define CARD_H   90
#define CARD_GAP 8

_Static_assert(WIDTH == AZMI_CONTENT_W, "Regenerate profile geometry");
_Static_assert(CARD_H == AZMI_PERIOD_H + AZMI_ENTRY_LINES * AZMI_LINE_H +
               AZMI_CARD_PAD_Y * 2, "Card geometry must match generated text");
_Static_assert(CARD_H * AZMI_ITEMS_PER_PAGE + CARD_GAP <= BODY_H,
               "Cards must fit above the navigation footer");

typedef struct {
    const char *title;
    const char *caption;
    uint32_t accent;
    uint32_t tint;
} page_theme_t;

static const page_theme_t THEMES[PASSPORT_PAGE_COUNT] = {
    {"Hi, Dr. Azmi!", "Big ideas. Big heart.", 0xAD4936, 0xFBE2D5},
    {"Meet Azmi", "Curiosity, always.", 0x23776F, 0xDDEFE8},
    {"The journey", "Every chapter builds the next.", 0x365DA0, 0xE6EBFA},
    {"XR explorer", "Ideas that change lives.", 0x7654A4, 0xEEE4F8},
    {"Well earned!", "Good ideas deserve a little gold.", 0x93601E, 0xFFF0CB},
    {"Real impact", "Making a difference.", 0x237B7C, 0xDDF0ED},
    {"Pass it on", "One teacher. Many sparks.", 0xA34565, 0xF8E2EA},
    {"Say hello!", "Scan for the full story.", 0x365DA0, 0xE6EBFA},
};

const uint8_t PASSPORT_SUBPAGES[PASSPORT_PAGE_COUNT] = {
    1, 1, AZMI_JOURNEY_PAGE_COUNT, AZMI_RESEARCH_PAGE_COUNT,
    AZMI_AWARDS_PAGE_COUNT, AZMI_IMPACT_PAGE_COUNT,
    AZMI_TEACHING_PAGE_COUNT, 1,
};

static lv_obj_t *s_screen;
static lv_obj_t *s_body;
static lv_obj_t *s_title;
static lv_obj_t *s_caption;
static lv_obj_t *s_counter;
static lv_obj_t *s_hint;
static lv_obj_t *s_battery;
static lv_obj_t *s_dots[PASSPORT_PAGE_COUNT];
static lv_obj_t *s_avatar;
static lv_obj_t *s_spark[3];
static lv_obj_t *s_message;
static passport_art_t s_art;
static uint8_t s_page = PASSPORT_PAGE_COUNT;
static uint32_t s_cheers;
static uint16_t s_qr_pixels[AZMI_QR_SIDE * AZMI_QR_SIDE];

static lv_obj_t *box(lv_obj_t *parent, int x, int y, int w, int h,
                     uint32_t color, int radius) {
    lv_obj_t *obj = lv_obj_create(parent);
    lv_obj_remove_style_all(obj);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, w, h);
    lv_obj_set_style_bg_color(obj, lv_color_hex(color), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(obj, radius, 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    return obj;
}

static lv_obj_t *label(lv_obj_t *parent, const char *text, int x, int y,
                       int w, const lv_font_t *font, uint32_t color) {
    lv_obj_t *obj = lv_label_create(parent);
    lv_label_set_text(obj, text);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_width(obj, w);
    lv_obj_set_style_text_font(obj, font, 0);
    lv_obj_set_style_text_color(obj, lv_color_hex(color), 0);
    lv_obj_set_style_text_line_space(obj, 2, 0);
    lv_label_set_long_mode(obj, LV_LABEL_LONG_WRAP);
    return obj;
}

static lv_obj_t *center_label(lv_obj_t *parent, const char *text, int y,
                              const lv_font_t *font, uint32_t color) {
    lv_obj_t *obj = label(parent, text, 0, y, WIDTH, font, color);
    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, 0);
    return obj;
}

static void translate_x(void *obj, int32_t value) {
    lv_obj_set_style_translate_x(obj, value, 0);
}

static void translate_y(void *obj, int32_t value) {
    lv_obj_set_style_translate_y(obj, value, 0);
}

static void motion(lv_obj_t *obj, lv_anim_exec_xcb_t exec, int start, int end,
                   uint32_t duration, uint32_t delay, bool playful) {
    lv_anim_delete(obj, exec);
    exec(obj, start);
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, obj);
    lv_anim_set_exec_cb(&anim, exec);
    lv_anim_set_values(&anim, start, end);
    lv_anim_set_duration(&anim, duration);
    lv_anim_set_delay(&anim, delay);
    lv_anim_set_path_cb(&anim, playful ? lv_anim_path_overshoot : lv_anim_path_ease_out);
    lv_anim_start(&anim);
}

static void float_briefly(lv_obj_t *obj, int distance, uint32_t delay) {
    lv_anim_delete(obj, translate_y);
    translate_y(obj, 0);
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, obj);
    lv_anim_set_exec_cb(&anim, translate_y);
    lv_anim_set_values(&anim, 0, -distance);
    lv_anim_set_duration(&anim, 650);
    lv_anim_set_delay(&anim, delay);
    lv_anim_set_playback_duration(&anim, 650);
    lv_anim_set_repeat_count(&anim, 2);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in_out);
    lv_anim_start(&anim);
}

static void build_portrait(void) {
    const page_theme_t *theme = &THEMES[PASSPORT_PAGE_PORTRAIT];
    lv_obj_t *mat = box(s_body, 18, 0, 148, 148, theme->tint, 18);
    s_avatar = lv_image_create(mat);
    lv_image_set_src(s_avatar, &AZMI_AVATAR_IMAGE);
    lv_obj_set_pos(s_avatar, 2, 2);
    static const int sx[] = {0, 162, 4};
    static const int sy[] = {12, 42, 112};
    static const uint32_t colors[] = {0xB67A18, 0x23776F, 0xA34565};
    for (int i = 0; i < 3; ++i) {
        s_spark[i] = label(s_body, "+", sx[i], sy[i], 20,
                           &lv_font_montserrat_20, colors[i]);
    }
    lv_obj_t *pill = box(s_body, 18, 153, 148, 23, theme->tint, 11);
    lv_obj_t *caption = label(pill, "BEYOND REALITY", 0, 4, 148,
                              &lv_font_montserrat_12, theme->accent);
    lv_obj_set_style_text_align(caption, LV_TEXT_ALIGN_CENTER, 0);
    s_message = center_label(s_body, "Thanks for inspiring us!", 175,
                             &lv_font_montserrat_12, MUTED);
    float_briefly(s_avatar, 3, 300);
    for (int i = 0; i < 3; ++i) float_briefly(s_spark[i], 5 + i, i * 120);
}

static void build_profile(void) {
    const page_theme_t *theme = &THEMES[PASSPORT_PAGE_PROFILE];
    label(s_body, AZMI_NAME_DISPLAY, 0, 0, WIDTH, &lv_font_montserrat_16, INK);
    label(s_body, AZMI_TITLE, 0, 43, WIDTH, &lv_font_montserrat_14, theme->accent);
    label(s_body, AZMI_INST_LINE1 "\n" AZMI_INST_LINE2, 0, 67, WIDTH,
          &lv_font_montserrat_14, MUTED);
    for (int i = 0; i < AZMI_TAG_COUNT && i < 4; ++i) {
        lv_obj_t *pill = box(s_body, i * 47, 111, 43, 27, theme->tint, 9);
        lv_obj_t *tag = label(pill, AZMI_TAGS[i], 0, 5, 43,
                              &lv_font_montserrat_14, theme->accent);
        lv_obj_set_style_text_align(tag, LV_TEXT_ALIGN_CENTER, 0);
    }
    lv_obj_t *office = box(s_body, 0, 149, WIDTH, 39, 0xFFFFFF, 10);
    label(office, "OFFICE", 12, 13, 65, &lv_font_montserrat_12, MUTED);
    lv_obj_t *value = label(office, AZMI_OFFICE, 88, 11, 84,
                            &lv_font_montserrat_14, INK);
    lv_obj_set_style_text_align(value, LV_TEXT_ALIGN_RIGHT, 0);
    // Email has a dedicated two-line block on the contact page.
}

static void build_cards(const char *const *periods,
                         const char *const (*lines)[AZMI_ENTRY_LINES]) {
    const page_theme_t *theme = &THEMES[s_page];
    for (int i = 0; i < AZMI_ITEMS_PER_PAGE; ++i) {
        if (!periods[i] || !periods[i][0]) continue;
        lv_obj_t *card = box(s_body, 0, i * (CARD_H + CARD_GAP), WIDTH,
                             CARD_H, i == 0 ? theme->tint : 0xFFFFFF, 12);
        box(card, 0, 12, 3, CARD_H - 24, theme->accent, 1);
        label(card, periods[i], 12, 6, 160, &lv_font_montserrat_14, theme->accent);
        for (int row = 0; row < AZMI_ENTRY_LINES; ++row) {
            label(card, lines[i][row], 12, 25 + row * AZMI_LINE_H, 160,
                  &lv_font_montserrat_14, row == 0 ? INK : MUTED);
        }
        motion(card, translate_y, 5, 0, 300, i * 55, true);
    }
    if (!periods[1] || !periods[1][0]) {
        center_label(s_body, "A well-deserved moment.", 119,
                      &lv_font_montserrat_12, theme->accent);
        center_label(s_body, "Keep exploring!", 143,
                      &lv_font_montserrat_16, INK);
    }
}

static void build_contact(void) {
    lv_obj_t *frame = box(s_body, (WIDTH - AZMI_QR_SIDE) / 2 - 4, 0,
                           AZMI_QR_SIDE + 8, AZMI_QR_SIDE + 8, 0xFFFFFF, 8);
    lv_obj_t *canvas = lv_canvas_create(frame);
    lv_canvas_set_buffer(canvas, s_qr_pixels, AZMI_QR_SIDE, AZMI_QR_SIDE,
                         LV_COLOR_FORMAT_RGB565);
    lv_obj_set_pos(canvas, 4, 4);
    center_label(s_body, AZMI_EMAIL_L1, 148, &lv_font_montserrat_14, INK);
    center_label(s_body, AZMI_EMAIL_L2, 167, &lv_font_montserrat_14, MUTED);
}

static void prepare_qr(void) {
    for (int y = 0; y < AZMI_QR_SIDE; ++y) {
        for (int x = 0; x < AZMI_QR_SIDE; ++x) {
            const int r = y / AZMI_QR_SCALE - AZMI_QR_QUIET;
            const int c = x / AZMI_QR_SCALE - AZMI_QR_QUIET;
            const bool dark = r >= 0 && r < QR_MODULE_COUNT && c >= 0 &&
                              c < QR_MODULE_COUNT && QR_MODULES[r][c];
            s_qr_pixels[y * AZMI_QR_SIDE + x] = dark ? 0 : 0xFFFF;
        }
    }
}

esp_err_t passport_ui_create(void) {
    if (s_screen) return ESP_OK;
    s_screen = box(NULL, 0, 0, 240, 320, BG, 0);
    label(s_screen, "AZMI / POCKET EDITION", LEFT, 14, 150,
          &lv_font_montserrat_12, MUTED);
    s_battery = label(s_screen, "--", 180, 14, 32, &lv_font_montserrat_12, MUTED);
    lv_obj_set_style_text_align(s_battery, LV_TEXT_ALIGN_RIGHT, 0);
    // Reserve a separate illustration slot; text never flows underneath it.
    s_title = label(s_screen, "", LEFT, 35, 138, &lv_font_montserrat_20, INK);
    s_caption = label(s_screen, "", LEFT, 61, 138, &lv_font_montserrat_12, MUTED);
    s_body = box(s_screen, LEFT, BODY_Y, WIDTH, BODY_H, BG, 0);
    box(s_screen, LEFT, 280, WIDTH, 1, RULE, 0);
    s_counter = label(s_screen, "", LEFT, 286, 47, &lv_font_montserrat_12, MUTED);
    s_hint = label(s_screen, "", 79, 286, 133, &lv_font_montserrat_12, INK);
    lv_obj_set_style_text_align(s_hint, LV_TEXT_ALIGN_RIGHT, 0);
    for (int i = 0; i < PASSPORT_PAGE_COUNT; ++i) {
        s_dots[i] = box(s_screen, 81 + i * 10, 308, 5, 3, RULE, 2);
    }
    prepare_qr();
    lv_screen_load(s_screen);
    const passport_nav_t nav = {.page = PASSPORT_PAGE_PORTRAIT};
    passport_ui_render(&nav);
    return ESP_OK;
}

void passport_ui_render(const passport_nav_t *nav) {
    if (!s_screen || !nav || nav->page >= PASSPORT_PAGE_COUNT) return;
    const uint8_t old_page = s_page;
    const bool backwards = old_page < PASSPORT_PAGE_COUNT && nav->page != old_page &&
        nav->page == (old_page + PASSPORT_PAGE_COUNT - 1) % PASSPORT_PAGE_COUNT;
    // Deleting children also removes their animations, including a cheer that
    // was interrupted by a page flip. Cancel persistent body motion explicitly.
    lv_anim_delete(s_body, NULL);
    lv_obj_clean(s_body);
    if (s_art.obj) lv_obj_delete(s_art.obj);
    s_avatar = s_message = NULL;
    for (int i = 0; i < 3; ++i) s_spark[i] = NULL;
    s_page = nav->page;
    const page_theme_t *theme = &THEMES[s_page];
    passport_art_create(&s_art, s_screen, (passport_page_t)s_page, 172, 34,
                         theme->accent, theme->tint);
    lv_label_set_text(s_title, theme->title);
    lv_obj_set_style_text_color(s_title, lv_color_hex(theme->accent), 0);
    const uint8_t count = PASSPORT_SUBPAGES[s_page];
    const unsigned sub = nav->subpage < count ? nav->subpage : 0;
    const char *caption = theme->caption;
    switch (s_page) {
        case PASSPORT_PAGE_PORTRAIT: build_portrait(); break;
        case PASSPORT_PAGE_PROFILE: build_profile(); break;
        case PASSPORT_PAGE_JOURNEY:
            caption = AZMI_JOURNEY_PAGE_TITLES[sub];
            build_cards(AZMI_JOURNEY_PERIODS[sub], AZMI_JOURNEY_LINES[sub]); break;
        case PASSPORT_PAGE_RESEARCH:
            caption = AZMI_RESEARCH_PAGE_TITLES[sub];
            build_cards(AZMI_RESEARCH_PERIODS[sub], AZMI_RESEARCH_LINES[sub]); break;
        case PASSPORT_PAGE_AWARDS:
            caption = AZMI_AWARDS_PAGE_TITLES[sub];
            build_cards(AZMI_AWARDS_PERIODS[sub], AZMI_AWARDS_LINES[sub]); break;
        case PASSPORT_PAGE_IMPACT:
            caption = AZMI_IMPACT_PAGE_TITLES[sub];
            build_cards(AZMI_IMPACT_PERIODS[sub], AZMI_IMPACT_LINES[sub]); break;
        case PASSPORT_PAGE_TEACHING:
            caption = AZMI_TEACHING_PAGE_TITLES[sub];
            build_cards(AZMI_TEACHING_PERIODS[sub], AZMI_TEACHING_LINES[sub]); break;
        case PASSPORT_PAGE_CONTACT: build_contact(); break;
        default: break;
    }
    lv_label_set_text(s_caption, caption);
    char counter[16];
    snprintf(counter, sizeof(counter), "%02u / %02u", s_page + 1u, PASSPORT_PAGE_COUNT);
    lv_label_set_text(s_counter, counter);
    char hint[32];
    if (count > 1) snprintf(hint, sizeof(hint), "UP/DN  OK %u/%u", sub + 1u, count);
    else snprintf(hint, sizeof(hint), "%s", s_page == PASSPORT_PAGE_PORTRAIT ?
                  "UP/DN  OK: JOY" : "UP/DN: EXPLORE");
    lv_label_set_text(s_hint, hint);
    for (int i = 0; i < PASSPORT_PAGE_COUNT; ++i) {
        lv_obj_set_style_bg_color(s_dots[i], lv_color_hex(i == s_page ? theme->accent : RULE), 0);
    }
    // No whole-container opacity: it needs a large intermediate RAM layer.
    if (old_page != PASSPORT_PAGE_COUNT) motion(s_body, translate_x, backwards ? -10 : 10,
                                                0, 220, 0, false);
    else translate_x(s_body, 0);
}

void passport_ui_celebrate(void) {
    if (s_page != PASSPORT_PAGE_PORTRAIT || !s_avatar || !s_message) return;
    static const char *const messages[] = {
        "Thanks for inspiring us!", "Your ideas open new worlds!",
        "A teacher worth celebrating!", "You make futures brighter!",
    };
    s_cheers = (s_cheers + 1) % (sizeof(messages) / sizeof(messages[0]));
    lv_label_set_text(s_message, messages[s_cheers]);
    passport_art_play(&s_art);
    float_briefly(s_avatar, 4, 0);
    for (int i = 0; i < 3; ++i) float_briefly(s_spark[i], 7 + i, i * 90);
}

void passport_ui_set_battery(int percent) {
    if (!s_battery) return;
    if (percent < 0 || percent > 100) lv_label_set_text(s_battery, "--");
    else lv_label_set_text_fmt(s_battery, "%d%%", percent);
}

void passport_ui_destroy(void) {
    if (!s_screen) return;
    lv_obj_delete(s_screen);
    s_screen = s_body = s_title = s_caption = s_counter = s_hint = s_battery = NULL;
    s_avatar = s_message = NULL;
    for (int i = 0; i < 3; ++i) s_spark[i] = NULL;
    s_page = PASSPORT_PAGE_COUNT;
}
