#pragma once
#include "esp_err.h"

// Call once from app_main after UI/input initialization, never under LVGL lock.
// Starts a one-shot worker; repeated calls do not replay. Failure is nonfatal.
// This is the application's sole audio owner; future players must serialize.
esp_err_t startup_sound_start(void);
