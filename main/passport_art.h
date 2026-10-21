#pragma once
#include "lvgl.h"
#include "passport_nav.h"

// A small native illustration: no decoded frame buffers or periodic worker.
// Own the state for at least as long as the object. All calls use the LVGL lock.
typedef struct {
    lv_obj_t *obj;
    passport_page_t page;
    uint32_t accent;
    uint8_t frame;
} passport_art_t;

void passport_art_create(passport_art_t *art, lv_obj_t *parent,
                         passport_page_t page, int x, int y,
                         uint32_t accent, uint32_t tint);
void passport_art_play(passport_art_t *art);
