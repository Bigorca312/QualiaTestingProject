#pragma once
#include "Globals.h"


#define BL_PIN A1                        // Backlight DIM pin (after soldering PWM jumper)
#define MIN_BACKLIGHT_PERCENT 20         // Minimum backlight brightness percent
#define BL_CH 0       // LEDC channel 0..7
#define BL_FREQ 5000  // pwm frequency in Hz
#define BL_RES 12     // 12-bit resolution (0..4095)
#define BL_INVERT 0   // set to 1 if brightness is inverted

namespace BacklightControl {
extern int CurrentBacklightPercent;

void initDynamicBacklightControl();
void set_backlight_percent(uint8_t pct);
}