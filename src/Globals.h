#pragma once
#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <lvgl.h>

//   0 = false, 1 = true
#define DISPLAY_TESTING_MODE 1 // keep this to 1 for this project (enables color fade test and other testing utilities)

// PARTIAL works (with jitters)
// DIRECT doesnt work
// FULL works (with jitters, but less frequent than partial)
#define DISPLAY_RENDER_MODE 0 // 0 = partial, 1 = direct, 2 = full
#define DOUBLE_BUFFERING    0 // NOTE: double buffer only works in DIRECT and FULL


// -------------------- Backlight (PWM on Qualia A1) --------------------
#define ENABLE_PWM_BACKLIGHT 1             // Backlight pin PWM enabled (after soldering PWM jumper)

extern Arduino_XCA9554SWSPI *expander;
