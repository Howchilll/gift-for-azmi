// main/passport_nav.c —— 导航状态机实现。纯 C,不依赖 ESP-IDF/LVGL。
#include "passport_nav.h"

#include <stdio.h>

void passport_nav_init(passport_nav_t *nav) {
    nav->page = PASSPORT_PAGE_PORTRAIT;
    nav->subpage = 0;
    nav->dirty = true;
}

// PROFILE 与 CONTACT 是单页;JOURNEY 与 RESEARCH 各有多个内容子页,
// 由 UI 层通过 PASSPORT_SUBPAGES 表提供真实数量。
extern const uint8_t PASSPORT_SUBPAGES[PASSPORT_PAGE_COUNT];

uint8_t passport_nav_subpage_count(const passport_nav_t *nav) {
    if (nav->page >= PASSPORT_PAGE_COUNT) return 1;
    const uint8_t count = PASSPORT_SUBPAGES[nav->page];
    return count == 0 ? 1 : count;
}

bool passport_nav_handle(passport_nav_t *nav, passport_input_t input) {
    switch (input) {
        case PASSPORT_INPUT_PREV:
            // UP 只翻页面,不碰子页。进入新页面时落在该页第一个子页。
            nav->page = (uint8_t)((nav->page + PASSPORT_PAGE_COUNT - 1) % PASSPORT_PAGE_COUNT);
            nav->subpage = 0;
            nav->dirty = true;
            return true;

        case PASSPORT_INPUT_NEXT:
            // DOWN 只翻页面,顺序 PROFILE -> JOURNEY -> RESEARCH -> CONTACT -> PROFILE。
            nav->page = (uint8_t)((nav->page + 1) % PASSPORT_PAGE_COUNT);
            nav->subpage = 0;
            nav->dirty = true;
            return true;

        case PASSPORT_INPUT_OK:
            // OK 只在当前页内切换内容子页;单页面上不做操作。
            {
                const uint8_t count = passport_nav_subpage_count(nav);
                if (count < 2) return false;
                nav->subpage = (uint8_t)((nav->subpage + 1) % count);
                nav->dirty = true;
                return true;
            }

        case PASSPORT_INPUT_NONE:
        default:
            return false;
    }
}

const char *passport_nav_page_label(const passport_nav_t *nav, char *buf, uint8_t buf_size) {
    if (!buf || buf_size == 0) return "";
    snprintf(buf, buf_size, "%02u / %02u",
             (unsigned)nav->page + 1u, (unsigned)PASSPORT_PAGE_COUNT);
    return buf;
}