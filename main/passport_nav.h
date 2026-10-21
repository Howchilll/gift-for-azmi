// main/passport_nav.h —— 纯逻辑导航状态机。不依赖 ESP-IDF / LVGL，可在主机上单测。
#pragma once

#include <stdbool.h>
#include <stdint.h>

// 页面顺序即翻页顺序,循环。
typedef enum {
    PASSPORT_PAGE_PORTRAIT = 0,  // 头像独占一页
    PASSPORT_PAGE_PROFILE,
    PASSPORT_PAGE_JOURNEY,
    PASSPORT_PAGE_RESEARCH,
    PASSPORT_PAGE_AWARDS,
    PASSPORT_PAGE_IMPACT,
    PASSPORT_PAGE_TEACHING,
    PASSPORT_PAGE_CONTACT,
    PASSPORT_PAGE_COUNT,
} passport_page_t;

// 归一化后的按键意图。UI 层负责把 BSP 事件翻译成这三种。
typedef enum {
    PASSPORT_INPUT_NONE = 0,
    PASSPORT_INPUT_PREV,  // UP   -> 上一页
    PASSPORT_INPUT_NEXT,  // DOWN -> 下一页
    PASSPORT_INPUT_OK,    // OK   -> 切换子页
} passport_input_t;

typedef struct {
    uint8_t page;     // 当前页面索引
    uint8_t subpage;  // 当前页内的子页索引
    bool    dirty;    // 置位表示界面需要按当前状态重绘
} passport_nav_t;

void passport_nav_init(passport_nav_t *nav);

// 处理一次输入。返回 true 表示状态发生变化,界面需要重绘。
bool passport_nav_handle(passport_nav_t *nav, passport_input_t input);

// 当前页面里可用的子页数量(由 UI 层提供的 PASSPORT_SUBPAGES 决定)。
uint8_t passport_nav_subpage_count(const passport_nav_t *nav);

// 供 UI 显示的页码文本,例如 "01 / 05"。调用方提供至少 8 字节缓冲。
const char *passport_nav_page_label(const passport_nav_t *nav, char *buf, uint8_t buf_size);