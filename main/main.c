// main/main.c —— AZMI ACADEMIC PASSPORT 应用入口。
//
// 本文件是派生应用,不复用上游的 demo 测试菜单与 ui_pixel 外壳:
// 界面全部由 main/passport_ui.c 实现,导航状态机在 main/passport_nav.c,
// 数据由 data/profile.json 生成。
//
// 按键语义(与 BSP 的 ADC 分压三键对应):
//   UP   短按 -> 上一页
//   DOWN 短按 -> 下一页
//   OK   短按 -> 当前页内切换内容子页；头像页触发庆祝动作与感谢语
#include "bsp_button.h"
#include "bsp_battery.h"
#include "bsp_display.h"
#include "bsp_pins.h"
#include "key_guard.h"
#include "lvgl.h"
#include "passport_nav.h"
#include "passport_ui.h"

#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

static const char *TAG = "passport";

#define INPUT_QUEUE_DEPTH 8
// 按键唤醒后那颗键可能仍被按住,用它挡住启动瞬间的误触发。
#define KEY_GUARD_MAX_US (10LL * 1000 * 1000)

typedef struct {
    bsp_btn_t btn;
    bsp_btn_ev_t event;
} input_event_t;

static passport_nav_t s_nav;
static QueueHandle_t s_input_queue;
static volatile bool s_input_ready;
static key_guard_t s_key_guard;
static bool s_battery_ready;

// 把 BSP 按键事件翻译成应用导航意图。只响应短按(CLICK)。
static passport_input_t navigation_input(bsp_btn_t btn, bsp_btn_ev_t ev) {
    if (ev != BSP_BTN_CLICK) return PASSPORT_INPUT_NONE;
    switch (btn) {
        case BSP_BTN_UP:   return PASSPORT_INPUT_PREV;
        case BSP_BTN_DOWN: return PASSPORT_INPUT_NEXT;
        case BSP_BTN_OK:   return PASSPORT_INPUT_OK;
        default:           return PASSPORT_INPUT_NONE;
    }
}

static void process_input(const input_event_t *input) {
    const passport_input_t nav_input = navigation_input(input->btn, input->event);
    if (nav_input == PASSPORT_INPUT_NONE) return;

    if (nav_input == PASSPORT_INPUT_OK && s_nav.page == PASSPORT_PAGE_PORTRAIT) {
        if (bsp_lvgl_lock(200)) {
            passport_ui_celebrate();
            bsp_lvgl_unlock();
        }
        return;
    }

    // 状态机本身是纯逻辑,可以在锁外计算;只有重绘需要 LVGL 锁。
    (void)passport_nav_handle(&s_nav, nav_input);
    if (!s_nav.dirty) return;

    if (!bsp_lvgl_lock(200)) return;
    passport_ui_render(&s_nav);
    s_nav.dirty = false;
    bsp_lvgl_unlock();
}

static void input_task(void *arg) {
    (void)arg;
    input_event_t input;
    int64_t battery_updated = 0;
    for (;;) {
        if (xQueueReceive(s_input_queue, &input, pdMS_TO_TICKS(1000)) == pdTRUE) {
            process_input(&input);
        }
        // Retry a failed UI lock on the next tick instead of leaving a dirty
        // state invisible until another navigation event arrives.
        if (s_input_ready && s_nav.dirty && bsp_lvgl_lock(200)) {
            passport_ui_render(&s_nav);
            s_nav.dirty = false;
            bsp_lvgl_unlock();
        }
        const int64_t now = esp_timer_get_time();
        if (s_input_ready && now - battery_updated >= 30LL * 1000 * 1000) {
            const int soc = s_battery_ready ? bsp_battery_soc() : -1;
            if (bsp_lvgl_lock(200)) {
                passport_ui_set_battery(soc);
                bsp_lvgl_unlock();
                battery_updated = now;
            }
        }
    }
}

static esp_err_t input_dispatch_init(void) {
    s_input_queue = xQueueCreate(INPUT_QUEUE_DEPTH, sizeof(input_event_t));
    if (!s_input_queue) return ESP_ERR_NO_MEM;
    if (xTaskCreate(input_task, "passport_input", 3072, NULL, 5, NULL) != pdPASS) {
        vQueueDelete(s_input_queue);
        s_input_queue = NULL;
        return ESP_ERR_NO_MEM;
    }
    return ESP_OK;
}

// button 回调运行在共享 esp_timer 任务上:只做入队,立即返回。
static void on_key(bsp_btn_t btn, bsp_btn_ev_t ev, void *user) {
    (void)user;
    const bool was_armed = s_key_guard.armed;
    if (key_guard_consume(&s_key_guard, ev == BSP_BTN_RELEASE, esp_timer_get_time())) {
        if (was_armed && !s_key_guard.armed) {
            ESP_LOGI(TAG, "唤醒键已松手,按键恢复响应");
        }
        return;
    }
    if (!s_input_ready || !s_input_queue) return;
    const input_event_t input = { .btn = btn, .event = ev };
    (void)xQueueSend(s_input_queue, &input, 0);
}

void app_main(void) {
    ESP_LOGI(TAG, "AZMI ACADEMIC PASSPORT 启动");

    const uint32_t wake_causes = esp_sleep_get_wakeup_causes();
    if (wake_causes & (1u << ESP_SLEEP_WAKEUP_GPIO)) {
        key_guard_arm(&s_key_guard, esp_timer_get_time(), KEY_GUARD_MAX_US);
        ESP_LOGI(TAG, "按键唤醒:先屏蔽按键动作,直到那颗键松手");
    }

    passport_nav_init(&s_nav);

    if (bsp_display_init() != ESP_OK || !bsp_lvgl_init()) {
        ESP_LOGE(TAG, "显示/LVGL 初始化失败。检查 SPI 接线"
                      "(MOSI=%d SCLK=%d CS=%d DC=%d BL=%d)",
                 BSP_LCD_MOSI, BSP_LCD_SCLK, BSP_LCD_CS, BSP_LCD_DC, BSP_LCD_BL);
        return;
    }
    bsp_display_backlight(100);

    s_battery_ready = bsp_battery_init() == ESP_OK;
    const int initial_soc = s_battery_ready ? bsp_battery_soc() : -1;

    const esp_err_t input_err = input_dispatch_init();
    if (input_err != ESP_OK) {
        ESP_LOGE(TAG, "输入任务创建失败: %s", esp_err_to_name(input_err));
        return;
    }

    const esp_err_t button_err = bsp_button_init(on_key, NULL);
    if (button_err != ESP_OK) {
        ESP_LOGE(TAG, "按键初始化失败: %s", esp_err_to_name(button_err));
        return;
    }

    if (s_key_guard.armed) {
        const bool released = bsp_button_read_mv() >= BSP_BTN_MV_RELEASED_MIN;
        key_guard_sync_released(&s_key_guard, released);
    }

    if (bsp_lvgl_lock(1000)) {
        passport_ui_create();
        passport_ui_set_battery(initial_soc);
        s_nav.dirty = false;
        bsp_lvgl_unlock();
        s_input_ready = true;
    } else {
        ESP_LOGE(TAG, "获取 LVGL 锁失败,界面未创建");
        return;
    }

    ESP_LOGI(TAG, "就绪:卡通头像首页,UP/DOWN 翻页,OK 切换子页或庆祝");
}
