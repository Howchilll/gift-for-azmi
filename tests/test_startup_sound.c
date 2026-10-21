// Execute the actual one-shot worker against a deterministic RTOS/audio boundary.
#include "demo_stubs/demo_test_stubs.h"
#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#include "../main/startup_sound.c"

static TaskFunction_t worker;
static jmp_buf exited;
static bool create_fail;
static unsigned creates, deletes, inits, formats, writes, sleeps, delay_ms;
static size_t samples_written;
static unsigned write_fail_at;
static esp_err_t init_result, format_result, sleep_result;
static uint8_t volume;

void test_log(const char *tag, const char *format, ...) { (void)tag; (void)format; }
const char *esp_err_to_name(esp_err_t error) { return error == ESP_OK ? "OK" : "FAIL"; }
BaseType_t xTaskCreate(TaskFunction_t entry, const char *name, unsigned stack,
                       void *arg, unsigned priority, TaskHandle_t *task) {
    assert(strcmp(name, "startup_sound") == 0 && stack == 3072 && priority == 5);
    assert(!arg && !task);
    creates++;
    if (create_fail) return pdFALSE;
    worker = entry;
    return pdPASS;
}
void vTaskDelete(TaskHandle_t task) { assert(!task); deletes++; longjmp(exited, 1); }
void vTaskDelay(TickType_t ticks) { delay_ms += ticks; }
esp_err_t bsp_audio_init(void) { inits++; return init_result; }
esp_err_t bsp_audio_set_format(uint32_t hz, uint8_t bits, uint8_t channels) {
    assert(hz == 16000 && bits == 16 && channels == 1 && volume == 0);
    formats++;
    return format_result;
}
void bsp_audio_set_volume(uint8_t percent) { assert(percent <= 55); volume = percent; }
esp_err_t bsp_audio_write(const void *pcm, size_t bytes) {
    assert(volume == 55 && bytes > 0 && bytes <= 512 && bytes % 2 == 0);
    assert(memcmp(pcm, startup_sound_pcm + samples_written, bytes) == 0);
    writes++;
    if (writes == write_fail_at) return ESP_FAIL;
    samples_written += bytes / 2;
    return ESP_OK;
}
esp_err_t bsp_audio_sleep(void) { assert(volume == 0); sleeps++; return sleep_result; }

static void reset(void) {
    s_started = false;
    worker = NULL;
    create_fail = false;
    creates = deletes = inits = formats = writes = sleeps = delay_ms = 0;
    samples_written = write_fail_at = volume = 0;
    init_result = format_result = sleep_result = ESP_OK;
}
static void run(void) {
    assert(worker);
    if (setjmp(exited) == 0) {
        worker(NULL);
        assert(!"one-shot task returned without deleting itself");
    }
    assert(deletes == 1);
}
int main(void) {
    reset();
    create_fail = true;
    assert(startup_sound_start() == ESP_ERR_NO_MEM && !s_started);
    create_fail = false;
    assert(startup_sound_start() == ESP_OK);
    assert(inits == 0); // Never perform blocking work in start()/app_main.
    assert(startup_sound_start() == ESP_OK && creates == 2);
    run();
    assert(inits == 1 && formats == 1 && sleeps == 1 && delay_ms == 100);
    assert(samples_written == STARTUP_SOUND_SAMPLES);
    assert(writes == (STARTUP_SOUND_SAMPLES + CHUNK_SAMPLES - 1) / CHUNK_SAMPLES);
    assert(startup_sound_start() == ESP_OK && creates == 2); // No replay after exit.

    reset(); init_result = ESP_FAIL;
    assert(startup_sound_start() == ESP_OK); run();
    assert(formats == 0 && writes == 0 && sleeps == 0);

    reset(); format_result = ESP_FAIL;
    assert(startup_sound_start() == ESP_OK); run();
    assert(writes == 0 && sleeps == 1 && delay_ms == 0);

    for (unsigned failure = 1; failure <= 3; failure += 2) {
        reset(); write_fail_at = failure;
        assert(startup_sound_start() == ESP_OK); run();
        assert(writes == failure && sleeps == 1 && delay_ms == 0);
    }
    reset(); sleep_result = ESP_FAIL;
    assert(startup_sound_start() == ESP_OK); run();
    assert(samples_written == STARTUP_SOUND_SAMPLES && sleeps == 1);
    puts("startup cue one-shot, stream, allocation and codec fault tests: PASS");
    return 0;
}
