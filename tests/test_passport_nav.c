// tests/test_passport_nav.c —— 导航状态机的主机端单元测试。
// 编译运行:
//   cc -std=c11 -Wall -Wextra -Imain tests/test_passport_nav.c main/passport_nav.c -o test_nav && ./test_nav
//
// 需求约定:UP=上一页,DOWN=下一页(只翻页面),OK=在当前页内切换内容子页。
#include <stdio.h>
#include <string.h>

#include "passport_nav.h"

// 与 UI 层一致的子页数量表(生成的头文件里是 4/4/3/3/3,这里手写以便脱离构建)。
const uint8_t PASSPORT_SUBPAGES[PASSPORT_PAGE_COUNT] = {
    [PASSPORT_PAGE_PORTRAIT] = 1,
    [PASSPORT_PAGE_PROFILE]  = 1,
    [PASSPORT_PAGE_JOURNEY]  = 4,
    [PASSPORT_PAGE_RESEARCH] = 4,
    [PASSPORT_PAGE_AWARDS]   = 3,
    [PASSPORT_PAGE_IMPACT]   = 3,
    [PASSPORT_PAGE_TEACHING] = 3,
    [PASSPORT_PAGE_CONTACT]  = 1,
};

static int failures;

#define CHECK(cond)                                                            \
    do {                                                                       \
        if (!(cond)) {                                                         \
            printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond);             \
            failures++;                                                        \
        }                                                                      \
    } while (0)

static void test_starts_on_portrait(void) {
    passport_nav_t nav;
    passport_nav_init(&nav);
    CHECK(nav.page == PASSPORT_PAGE_PORTRAIT);
    CHECK(nav.subpage == 0);
    CHECK(passport_nav_subpage_count(&nav) == 1);
}

static void test_down_walks_all_pages_and_wraps(void) {
    passport_nav_t nav;
    passport_nav_init(&nav);

    const passport_page_t order[] = {
        PASSPORT_PAGE_PROFILE, PASSPORT_PAGE_JOURNEY, PASSPORT_PAGE_RESEARCH,
        PASSPORT_PAGE_AWARDS, PASSPORT_PAGE_IMPACT, PASSPORT_PAGE_TEACHING,
        PASSPORT_PAGE_CONTACT,
    };
    for (size_t i = 0; i < sizeof(order) / sizeof(order[0]); i++) {
        passport_nav_handle(&nav, PASSPORT_INPUT_NEXT);
        CHECK(nav.page == order[i]);
    }
    passport_nav_handle(&nav, PASSPORT_INPUT_NEXT);
    CHECK(nav.page == PASSPORT_PAGE_PORTRAIT); // 循环回起点
}

static void test_up_wraps_to_contact(void) {
    passport_nav_t nav;
    passport_nav_init(&nav);
    passport_nav_handle(&nav, PASSPORT_INPUT_PREV);
    CHECK(nav.page == PASSPORT_PAGE_CONTACT);
    passport_nav_handle(&nav, PASSPORT_INPUT_PREV);
    CHECK(nav.page == PASSPORT_PAGE_TEACHING);
}

static void test_page_flip_resets_subpage(void) {
    passport_nav_t nav;
    passport_nav_init(&nav);
    passport_nav_handle(&nav, PASSPORT_INPUT_NEXT); // PORTRAIT -> PROFILE
    passport_nav_handle(&nav, PASSPORT_INPUT_NEXT); // PROFILE -> JOURNEY
    CHECK(nav.page == PASSPORT_PAGE_JOURNEY);
    passport_nav_handle(&nav, PASSPORT_INPUT_OK);
    passport_nav_handle(&nav, PASSPORT_INPUT_OK);
    CHECK(nav.subpage == 2);

    passport_nav_handle(&nav, PASSPORT_INPUT_NEXT); // -> RESEARCH
    CHECK(nav.page == PASSPORT_PAGE_RESEARCH);
    CHECK(nav.subpage == 0);
}

static void test_ok_noop_on_single_page(void) {
    passport_nav_t nav;
    passport_nav_init(&nav);

    CHECK(passport_nav_handle(&nav, PASSPORT_INPUT_OK) == false);
    CHECK(nav.page == PASSPORT_PAGE_PORTRAIT && nav.subpage == 0);

    passport_nav_handle(&nav, PASSPORT_INPUT_NEXT); // PROFILE
    CHECK(passport_nav_handle(&nav, PASSPORT_INPUT_OK) == false);
    CHECK(nav.subpage == 0);
}

static void test_ok_cycles_subpages(void) {
    passport_nav_t nav;
    passport_nav_init(&nav);
    passport_nav_handle(&nav, PASSPORT_INPUT_NEXT); // PORTRAIT -> PROFILE
    passport_nav_handle(&nav, PASSPORT_INPUT_NEXT); // PROFILE -> JOURNEY
    CHECK(nav.page == PASSPORT_PAGE_JOURNEY);

    for (int i = 1; i < 4; i++) {
        CHECK(passport_nav_handle(&nav, PASSPORT_INPUT_OK) == true);
        CHECK(nav.subpage == i);
    }
    CHECK(passport_nav_handle(&nav, PASSPORT_INPUT_OK) == true);
    CHECK(nav.subpage == 0); // 循环
}

static void test_teaching_has_three_subpages(void) {
    passport_nav_t nav;
    passport_nav_init(&nav);
    for (int i = 0; i < 6; i++) {   // PORTRAIT(0) -> ... -> TEACHING(6)
        passport_nav_handle(&nav, PASSPORT_INPUT_NEXT);
    }
    CHECK(nav.page == PASSPORT_PAGE_TEACHING);
    CHECK(passport_nav_subpage_count(&nav) == 3);
    CHECK(passport_nav_handle(&nav, PASSPORT_INPUT_OK) == true);
    CHECK(nav.subpage == 1);
}

static void test_none_is_noop(void) {
    passport_nav_t nav;
    passport_nav_init(&nav);
    CHECK(passport_nav_handle(&nav, PASSPORT_INPUT_NONE) == false);
    CHECK(nav.page == PASSPORT_PAGE_PORTRAIT && nav.subpage == 0);
}

static void test_dirty_flag_set_on_change(void) {
    passport_nav_t nav;
    passport_nav_init(&nav);
    nav.dirty = false;
    passport_nav_handle(&nav, PASSPORT_INPUT_NEXT);
    CHECK(nav.dirty == true);
}

static void test_page_label(void) {
    passport_nav_t nav;
    passport_nav_init(&nav);
    char buf[8];
    CHECK(strcmp(passport_nav_page_label(&nav, buf, sizeof(buf)), "01 / 08") == 0);
    for (int i = 0; i < 7; i++) {
        passport_nav_handle(&nav, PASSPORT_INPUT_NEXT);
    }
    CHECK(strcmp(passport_nav_page_label(&nav, buf, sizeof(buf)), "08 / 08") == 0);
}

static void test_stress_cycling(void) {
    passport_nav_t nav;
    passport_nav_init(&nav);
    for (int i = 0; i < 3000; i++) {
        const passport_input_t input = (passport_input_t)(i % 3);
        passport_nav_handle(&nav, input);
        CHECK(nav.page < PASSPORT_PAGE_COUNT);
        CHECK(nav.subpage < passport_nav_subpage_count(&nav));
    }
}

int main(void) {
    test_starts_on_portrait();
    test_down_walks_all_pages_and_wraps();
    test_up_wraps_to_contact();
    test_page_flip_resets_subpage();
    test_ok_noop_on_single_page();
    test_ok_cycles_subpages();
    test_teaching_has_three_subpages();
    test_none_is_noop();
    test_dirty_flag_set_on_change();
    test_page_label();
    test_stress_cycling();

    if (failures == 0) {
        printf("passport_nav: all tests passed\n");
        return 0;
    }
    printf("passport_nav: %d failure(s)\n", failures);
    return 1;
}