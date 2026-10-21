#include "startup_sound.h"
#include "startup_sound_data.h"
#include "bsp_audio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdbool.h>
#include <string.h>

static const char *TAG = "startup_sound";
static bool s_started;
#define CHUNK_SAMPLES 256u
#define STARTUP_VOLUME 55

static void startup_sound_task(void *arg) {
    (void)arg;
    esp_err_t error = bsp_audio_init();
    if (error != ESP_OK) {
        ESP_LOGW(TAG, "Audio unavailable; continuing silently: %s", esp_err_to_name(error));
        vTaskDelete(NULL);
        return;
    }
    // Mute before opening so codec setup cannot briefly restore a loud volume.
    bsp_audio_set_volume(0);
    error = bsp_audio_set_format(STARTUP_SOUND_RATE, 16, 1);
    if (error == ESP_OK) {
        bsp_audio_set_volume(STARTUP_VOLUME);
        int16_t chunk[CHUNK_SAMPLES];
        for (size_t offset = 0; offset < STARTUP_SOUND_SAMPLES;) {
            size_t count = STARTUP_SOUND_SAMPLES - offset;
            if (count > CHUNK_SAMPLES) count = CHUNK_SAMPLES;
            memcpy(chunk, startup_sound_pcm + offset, count * sizeof(*chunk));
            error = bsp_audio_write(chunk, count * sizeof(*chunk));
            if (error != ESP_OK) break;
            offset += count;
        }
        // BSP queues up to 90 ms at 16 kHz. The asset ends in 120 ms silence;
        // additionally wait for queued data before muting/suspending the codec.
        if (error == ESP_OK) vTaskDelay(pdMS_TO_TICKS(100));
    }
    if (error != ESP_OK) ESP_LOGW(TAG, "Cue skipped/interrupted: %s", esp_err_to_name(error));
    bsp_audio_set_volume(0);
    const esp_err_t sleep_error = bsp_audio_sleep();
    if (sleep_error != ESP_OK) {
        ESP_LOGW(TAG, "Codec suspend failed: %s", esp_err_to_name(sleep_error));
    }
    ESP_LOGI(TAG, "Cue finished (playback=%s, suspend=%s)",
             esp_err_to_name(error), esp_err_to_name(sleep_error));
    // No task handle, UI pointer or buffer survives this one-shot worker.
    vTaskDelete(NULL);
}

esp_err_t startup_sound_start(void) {
    if (s_started) return ESP_OK;
    // app_main is the sole caller. No state is touched by the worker.
    if (xTaskCreate(startup_sound_task, "startup_sound", 3072, NULL, 5, NULL) != pdPASS) {
        return ESP_ERR_NO_MEM;
    }
    s_started = true;
    return ESP_OK;
}
