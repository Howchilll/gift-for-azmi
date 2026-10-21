// Render the real firmware UI with the pinned LVGL, not a drawing imitation.
// Usage: passport_preview <existing-output-directory>
#include "lvgl.h"
#include "passport_ui.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint16_t framebuffer[240 * 320];
static uint16_t draw_buffer[240 * 40];
static unsigned errors;

static uint32_t sticker_hash(void) {
    uint32_t hash = 2166136261u;
    for (int y = 34; y < 76; ++y) {
        for (int x = 172; x < 212; ++x) {
            hash = (hash ^ framebuffer[y * 240 + x]) * 16777619u;
        }
    }
    return hash;
}

static void flush(lv_display_t *display, const lv_area_t *area, uint8_t *pixels) {
    const uint16_t *source = (const uint16_t *)pixels;
    for (int y = area->y1; y <= area->y2; ++y) {
        for (int x = area->x1; x <= area->x2; ++x) {
            framebuffer[y * 240 + x] = *source++;
        }
    }
    lv_display_flush_ready(display);
}

static void advance(unsigned ms) {
    for (unsigned elapsed = 0; elapsed < ms; elapsed += 20) {
        lv_tick_inc(20);
        lv_timer_handler();
    }
    lv_obj_update_layout(lv_screen_active());
    lv_refr_now(NULL);
}

static void save(const char *directory, const char *name) {
    char path[1024];
    snprintf(path, sizeof(path), "%s/%s.ppm", directory, name);
    FILE *file = fopen(path, "wb");
    assert(file);
    fprintf(file, "P6\n240 320\n255\n");
    for (int y = 0; y < 320; ++y) {
        for (int x = 0; x < 240; ++x) {
            // Same physical 30 px corner mask as the device.
            int dx = x < 30 ? 30 - x : (x >= 210 ? x - 209 : 0);
            int dy = y < 30 ? 30 - y : (y >= 290 ? y - 289 : 0);
            const uint16_t p = dx && dy && dx * dx + dy * dy > 900 ? 0 : framebuffer[y * 240 + x];
            fputc(((p >> 11) & 31) * 255 / 31, file);
            fputc(((p >> 5) & 63) * 255 / 63, file);
            fputc((p & 31) * 255 / 31, file);
        }
    }
    fclose(file);
}

static bool overlaps(const lv_area_t *a, const lv_area_t *b) {
    return a->x1 <= b->x2 && a->x2 >= b->x1 && a->y1 <= b->y2 && a->y2 >= b->y1;
}

static void check_labels(lv_obj_t *parent) {
    lv_area_t bounds;
    lv_obj_get_coords(parent, &bounds);
    for (unsigned i = 0; i < lv_obj_get_child_count(parent); ++i) {
        lv_obj_t *child = lv_obj_get_child(parent, i);
        if (lv_obj_check_type(child, &lv_label_class) && lv_label_get_text(child)[0]) {
            lv_area_t area;
            lv_obj_get_coords(child, &area);
            if (area.x1 < bounds.x1 || area.x2 > bounds.x2 ||
                area.y1 < bounds.y1 || area.y2 > bounds.y2) {
                fprintf(stderr, "Clipped label: '%s' [%ld,%ld..%ld,%ld] parent [%ld,%ld..%ld,%ld]\n",
                        lv_label_get_text(child), (long)area.x1, (long)area.y1, (long)area.x2, (long)area.y2,
                        (long)bounds.x1, (long)bounds.y1, (long)bounds.x2, (long)bounds.y2);
                ++errors;
            }
            if (parent == lv_screen_active() && area.y1 < 82 && area.y2 >= 82) {
                fprintf(stderr, "Header overlaps body: '%s'\n", lv_label_get_text(child));
                ++errors;
            }
            for (unsigned j = i + 1; j < lv_obj_get_child_count(parent); ++j) {
                lv_obj_t *other = lv_obj_get_child(parent, j);
                if (!lv_obj_check_type(other, &lv_label_class) || !lv_label_get_text(other)[0]) continue;
                lv_area_t other_area;
                lv_obj_get_coords(other, &other_area);
                if (overlaps(&area, &other_area)) {
                    fprintf(stderr, "Overlapping labels: '%s' / '%s'\n", lv_label_get_text(child), lv_label_get_text(other));
                    ++errors;
                }
            }
        }
        check_labels(child);
    }
}

int main(int argc, char **argv) {
    if (argc != 2) return 2;
    lv_init();
    lv_display_t *display = lv_display_create(240, 320);
    lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);
    lv_display_set_buffers(display, draw_buffer, NULL, sizeof(draw_buffer), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display, flush);
    if (passport_ui_create() != ESP_OK) return 3;
    passport_ui_set_battery(87);
    advance(4400);
    for (uint8_t page = 0; page < PASSPORT_PAGE_COUNT; ++page) {
        passport_nav_t nav = {.page = page};
        for (uint8_t sub = 0; sub < PASSPORT_SUBPAGES[page]; ++sub) {
            nav.subpage = sub;
            passport_ui_render(&nav);
            advance(4400);
            check_labels(lv_screen_active());
            char name[80];
            snprintf(name, sizeof(name), "page-%02u-%02u", page + 1, sub + 1);
            save(argv[1], name);
            if (sub == 0) {
                // Verify each chapter really changes its own illustration,
                // independently from text, avatar bobbing and page transitions.
                passport_ui_render(&nav);
                advance(400);
                uint32_t hashes[8];
                unsigned unique = 0;
                for (unsigned frame = 0; frame < 8; ++frame) {
                    advance(100);
                    hashes[frame] = sticker_hash();
                    bool seen = false;
                    for (unsigned j = 0; j < frame; ++j) {
                        if (hashes[j] == hashes[frame]) seen = true;
                    }
                    if (!seen) ++unique;
                    snprintf(name, sizeof(name), "section-%02u-frame-%02u", page + 1, frame);
                    save(argv[1], name);
                    check_labels(lv_screen_active());
                }
                if (unique < 3) {
                    fprintf(stderr, "Chapter %u illustration has only %u distinct frames\n", page, unique);
                    ++errors;
                }
                advance(4400);
                if (lv_anim_count_running() != 0) {
                    fprintf(stderr, "Chapter %u kept animating at idle\n", page);
                    ++errors;
                }
            }
        }
    }
    passport_nav_t portrait = {.page = PASSPORT_PAGE_PORTRAIT};
    passport_ui_render(&portrait);
    advance(4400);
    for (unsigned cheer = 0; cheer < 4; ++cheer) {
        passport_ui_celebrate();
        advance(4400);
        check_labels(lv_screen_active());
    }
    // Record real LVGL motion frames, including a page flip midway through cheer.
    passport_ui_celebrate();
    for (unsigned frame = 0; frame < 65; ++frame) {
        advance(40);
        char name[80];
        snprintf(name, sizeof(name), "motion-%03u", frame);
        save(argv[1], name);
    }
    for (unsigned frame = 0; frame < 16; ++frame) {
        if (frame == 0) { portrait.page = PASSPORT_PAGE_RESEARCH; passport_ui_render(&portrait); }
        advance(40);
        char name[80];
        snprintf(name, sizeof(name), "motion-%03u", frame + 65);
        save(argv[1], name);
    }
    advance(4400);
    lv_mem_monitor_t before, after;
    lv_mem_monitor(&before);
    for (unsigned i = 0; i < 500; ++i) {
        passport_nav_t nav = {.page = i % PASSPORT_PAGE_COUNT};
        nav.subpage = i % PASSPORT_SUBPAGES[nav.page];
        passport_ui_render(&nav);
        if (nav.page == PASSPORT_PAGE_PORTRAIT) passport_ui_celebrate();
        advance(20 + i % 9 * 20);
    }
    portrait.page = PASSPORT_PAGE_RESEARCH;
    passport_ui_render(&portrait);
    advance(4400);
    check_labels(lv_screen_active());
    lv_mem_monitor(&after);
    printf("LVGL pool: %lu bytes; before free %lu; after free %lu; max used %lu\n",
           (unsigned long)after.total_size, (unsigned long)before.free_size,
           (unsigned long)after.free_size, (unsigned long)after.max_used);
    if (after.free_size + 256 < before.free_size) { fprintf(stderr, "Possible page-cycle memory leak\n"); ++errors; }
    portrait.page = PASSPORT_PAGE_CONTACT;
    passport_ui_render(&portrait);
    advance(4400);
    if (lv_anim_count_running() != 0) { fprintf(stderr, "Idle animations did not stop\n"); ++errors; }
    passport_ui_destroy();
    if (lv_anim_count_running() != 0) { fprintf(stderr, "Animations survived destruction\n"); ++errors; }
    if (passport_ui_create() != ESP_OK) return 3;
    advance(4400);
    check_labels(lv_screen_active());
    printf("Real LVGL rendering: %u layout/lifecycle errors\n", errors);
    return errors ? 1 : 0;
}
