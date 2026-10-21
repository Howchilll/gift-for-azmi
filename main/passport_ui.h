#pragma once
#include "esp_err.h"
#include "passport_nav.h"

// All calls require bsp_lvgl_lock(). One live screen owns child animations.
esp_err_t passport_ui_create(void);
void passport_ui_render(const passport_nav_t *nav);
void passport_ui_celebrate(void);
void passport_ui_set_battery(int percent);
void passport_ui_destroy(void);
extern const uint8_t PASSPORT_SUBPAGES[PASSPORT_PAGE_COUNT];
