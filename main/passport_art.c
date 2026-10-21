// Eight tiny illustrated stories, each with an eight-frame movement cycle.
// Draw events use LVGL primitives directly into the normal partial DMA render.
// The 40x42 sticker has one object and no image/canvas RAM allocation.
#include "passport_art.h"

#define INK 0x354454
#define PAPER 0xFFFEFA
#define GOLD 0xF1B943
#define CORAL 0xDC775D
#define MINT 0x69B8A6

typedef struct {
    lv_layer_t *layer;
    int x;
    int y;
    uint32_t accent;
} pen_t;

static void rect(pen_t *p, int x, int y, int w, int h, int radius,
                  uint32_t fill, uint32_t stroke, int border) {
    lv_draw_rect_dsc_t d;
    lv_draw_rect_dsc_init(&d);
    d.bg_color = lv_color_hex(fill);
    d.bg_opa = LV_OPA_COVER;
    d.radius = radius;
    d.border_color = lv_color_hex(stroke);
    d.border_width = border;
    d.border_opa = LV_OPA_COVER;
    const lv_area_t area = {p->x + x, p->y + y, p->x + x + w - 1, p->y + y + h - 1};
    lv_draw_rect(p->layer, &d, &area);
}

static void line(pen_t *p, int x1, int y1, int x2, int y2, uint32_t color, int width) {
    lv_draw_line_dsc_t d;
    lv_draw_line_dsc_init(&d);
    d.p1 = (lv_point_precise_t){p->x + x1, p->y + y1};
    d.p2 = (lv_point_precise_t){p->x + x2, p->y + y2};
    d.color = lv_color_hex(color);
    d.width = width;
    d.round_start = d.round_end = true;
    lv_draw_line(p->layer, &d);
}

static void triangle(pen_t *p, int x1, int y1, int x2, int y2,
                      int x3, int y3, uint32_t color) {
    lv_draw_triangle_dsc_t d;
    lv_draw_triangle_dsc_init(&d);
    d.p[0] = (lv_point_precise_t){p->x + x1, p->y + y1};
    d.p[1] = (lv_point_precise_t){p->x + x2, p->y + y2};
    d.p[2] = (lv_point_precise_t){p->x + x3, p->y + y3};
    d.color = lv_color_hex(color);
    d.opa = LV_OPA_COVER;
    lv_draw_triangle(p->layer, &d);
}

static void arc(pen_t *p, int x, int y, int radius, int start,
                 int end, uint32_t color, int width) {
    lv_draw_arc_dsc_t d;
    lv_draw_arc_dsc_init(&d);
    d.center = (lv_point_t){p->x + x, p->y + y};
    d.radius = radius;
    d.start_angle = start;
    d.end_angle = end;
    d.color = lv_color_hex(color);
    d.width = width;
    d.rounded = true;
    lv_draw_arc(p->layer, &d);
}

static void sparkle(pen_t *p, int x, int y, int size, uint32_t color) {
    line(p, x - size, y, x + size, y, color, 1);
    line(p, x, y - size, x, y + size, color, 1);
}

static void heart(pen_t *p, int x, int y, uint32_t color) {
    rect(p, x - 5, y - 3, 6, 6, 3, color, color, 0);
    rect(p, x, y - 3, 6, 6, 3, color, color, 0);
    triangle(p, x - 5, y, x + 6, y, x, y + 7, color);
}

static void draw_story(lv_event_t *event) {
    passport_art_t *art = lv_event_get_user_data(event);
    lv_area_t area;
    lv_obj_get_coords(art->obj, &area);
    pen_t p = {lv_event_get_layer(event), area.x1, area.y1, art->accent};
    const unsigned f = art->frame;
    static const int bob[] = {0, -1, -2, -1, 0, 1, 2, 1};
    const int b = bob[f];
    switch (art->page) {
        case PASSPORT_PAGE_PORTRAIT:
            // A smiling sun: an affectionate greeting to the recipient.
            for (int i = 0; i < 4; ++i) {
                const int ray = 3 + ((f + i) % 3 == 0);
                if (i == 0) line(&p, 20, 5, 20, 5 + ray, GOLD, 2);
                if (i == 1) line(&p, 32, 20, 32 + ray, 20, GOLD, 2);
                if (i == 2) line(&p, 20, 32, 20, 32 + ray, GOLD, 2);
                if (i == 3) line(&p, 4, 20, 4 + ray, 20, GOLD, 2);
            }
            rect(&p, 10, 11, 21, 21, 11, GOLD, p.accent, 1);
            line(&p, 15, 19, 15, f == 4 ? 19 : 20, INK, 2);
            line(&p, 25, 19, 25, f == 4 ? 19 : 20, INK, 2);
            arc(&p, 20, 23, 5, 15, 165, INK, 1);
            break;
        case PASSPORT_PAGE_PROFILE:
            // An explorer's ID card, with a waving corner sparkle.
            rect(&p, 6, 9, 28, 25, 5, PAPER, p.accent, 1);
            rect(&p, 10, 14, 9, 9, 5, GOLD, GOLD, 0);
            rect(&p, 8, 24, 13, 6, 4, MINT, MINT, 0);
            line(&p, 24, 17, 30, 17, p.accent, 2);
            line(&p, 24, 23, 29, 23, p.accent, 1);
            sparkle(&p, 32, 8, 2 + (f % 3), CORAL);
            break;
        case PASSPORT_PAGE_JOURNEY:
            // A winding route with a travelling dot and a summit flag.
            line(&p, 8, 33, 8, 24, p.accent, 2);
            arc(&p, 15, 24, 7, 180, 360, p.accent, 2);
            arc(&p, 22, 24, 7, 0, 180, p.accent, 2);
            line(&p, 29, 24, 29, 11, p.accent, 2);
            line(&p, 29, 11, 29, 5, INK, 1);
            triangle(&p, 30, 5, 36, 8, 30, 11, CORAL);
            {
                static const int xs[] = {8, 8, 12, 17, 21, 26, 29, 29};
                static const int ys[] = {33, 25, 18, 18, 28, 28, 24, 16};
                rect(&p, xs[f] - 3, ys[f] - 3, 7, 7, 4, GOLD, INK, 1);
            }
            break;
        case PASSPORT_PAGE_RESEARCH:
            // VR goggles smile; a tiny signal travels between the lenses.
            line(&p, 5, 20 + b, 9, 20 + b, INK, 3);
            line(&p, 31, 20 + b, 35, 20 + b, INK, 3);
            rect(&p, 8, 12 + b, 25, 18, 6, p.accent, INK, 1);
            rect(&p, 12, 16 + b, 8, 9, 3, PAPER, PAPER, 0);
            rect(&p, 22, 16 + b, 7, 9, 3, PAPER, PAPER, 0);
            rect(&p, 14 + f % 3, 19 + b, 2, 3, 1, MINT, MINT, 0);
            rect(&p, 24 + f % 2, 19 + b, 2, 3, 1, MINT, MINT, 0);
            sparkle(&p, 8, 7, 2, GOLD);
            sparkle(&p, 33, 34, 1 + f % 3, CORAL);
            break;
        case PASSPORT_PAGE_AWARDS:
            // A little gold cup bounces as its stars twinkle.
            arc(&p, 10, 16 + b, 6, 70, 280, p.accent, 2);
            arc(&p, 30, 16 + b, 6, 260, 110, p.accent, 2);
            rect(&p, 11, 10 + b, 19, 17, 6, GOLD, p.accent, 1);
            line(&p, 20, 26 + b, 20, 32, p.accent, 3);
            rect(&p, 12, 32, 17, 4, 2, p.accent, p.accent, 0);
            sparkle(&p, 20, 18 + b, 3, PAPER);
            sparkle(&p, 32, 6, 1 + f % 3, CORAL);
            break;
        case PASSPORT_PAGE_IMPACT:
            // A warm heart sends expanding ripples into the world.
            arc(&p, 20, 23, 12 + f / 2, 0, 360, MINT, 1);
            arc(&p, 20, 23, 8 + f / 3, 0, 360, p.accent, 1);
            heart(&p, 20, 20 + b / 2, CORAL);
            break;
        case PASSPORT_PAGE_TEACHING:
            // An open book turns one page while a new idea lights up above it.
            rect(&p, 6, 19, 14, 15, 3, PAPER, p.accent, 1);
            rect(&p, 20, 19, 14, 15, 3, PAPER, p.accent, 1);
            line(&p, 20, 20, 20, 35, p.accent, 1);
            line(&p, 10, 24, 16, 24, MINT, 1);
            line(&p, 24, 25, 30, 25, MINT, 1);
            if (f >= 2 && f <= 5) {
                triangle(&p, 20, 20, 20, 33, 26 - f, 17, GOLD);
            }
            rect(&p, 16, 6 + b, 9, 9, 5, GOLD, GOLD, 0);
            line(&p, 18, 16 + b, 22, 16 + b, p.accent, 2);
            sparkle(&p, 31, 9, 1 + f % 3, CORAL);
            break;
        case PASSPORT_PAGE_CONTACT:
            // An envelope opens and sends a tiny heart. The QR itself is static.
            rect(&p, 7, 16, 27, 18, 4, PAPER, p.accent, 1);
            line(&p, 8, 32, 19, 23, p.accent, 1);
            line(&p, 32, 32, 22, 23, p.accent, 1);
            if (f >= 2 && f <= 5) {
                triangle(&p, 8, 17, 20, 8, 33, 17, PAPER);
                line(&p, 8, 17, 20, 8, p.accent, 1);
                line(&p, 20, 8, 33, 17, p.accent, 1);
                heart(&p, 20, 9 - (int)(f - 2), CORAL);
            } else {
                line(&p, 8, 17, 20, 26, p.accent, 1);
                line(&p, 20, 26, 33, 17, p.accent, 1);
                sparkle(&p, 30, 9, 2, GOLD);
            }
            break;
        default: break;
    }
}

static void set_frame(void *obj, int32_t value) {
    passport_art_t *art = lv_obj_get_user_data(obj);
    const uint8_t frame = (uint8_t)value % 8;
    if (art->frame == frame) return;
    art->frame = frame;
    lv_obj_invalidate(obj);
}

static void deleted(lv_event_t *event) {
    passport_art_t *art = lv_event_get_user_data(event);
    art->obj = NULL;
}

void passport_art_play(passport_art_t *art) {
    if (!art || !art->obj) return;
    lv_anim_delete(art->obj, set_frame);
    set_frame(art->obj, 0);
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, art->obj);
    lv_anim_set_exec_cb(&anim, set_frame);
    // Four short eight-frame cycles at 10 fps; settle on frame zero at idle.
    lv_anim_set_values(&anim, 0, 32);
    lv_anim_set_duration(&anim, 3200);
    lv_anim_set_path_cb(&anim, lv_anim_path_linear);
    lv_anim_start(&anim);
}

void passport_art_create(passport_art_t *art, lv_obj_t *parent,
                         passport_page_t page, int x, int y,
                         uint32_t accent, uint32_t tint) {
    *art = (passport_art_t){.page = page, .accent = accent};
    art->obj = lv_obj_create(parent);
    lv_obj_remove_style_all(art->obj);
    lv_obj_set_pos(art->obj, x, y);
    lv_obj_set_size(art->obj, 40, 42);
    lv_obj_set_style_bg_color(art->obj, lv_color_hex(tint), 0);
    lv_obj_set_style_bg_opa(art->obj, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(art->obj, 12, 0);
    lv_obj_clear_flag(art->obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_user_data(art->obj, art);
    lv_obj_add_event_cb(art->obj, draw_story, LV_EVENT_DRAW_MAIN, art);
    lv_obj_add_event_cb(art->obj, deleted, LV_EVENT_DELETE, art);
    passport_art_play(art);
}
