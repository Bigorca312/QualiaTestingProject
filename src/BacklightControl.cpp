#include "BacklightControl.h"

namespace BacklightControl {

int CurrentBacklightPercent = MIN_BACKLIGHT_PERCENT;

// Call after backlight_begin().
void set_backlight_percent(uint8_t pct) {
  if (pct > 100) pct = 100;
  CurrentBacklightPercent = pct;

  const float gamma = 2.2f;
  float lin = pct / 100.0f;
  float perceptual = powf(lin, gamma);
  uint32_t maxDuty = (1u << BL_RES) - 1u;
  uint32_t duty = (uint32_t)lroundf(perceptual * maxDuty);
  if (BL_INVERT) duty = maxDuty - duty;
  ledcWrite(BL_PIN, duty);
}

// Initialize the backlight with a starting brightness percentage (default 20%)
static inline void backlight_begin(uint8_t start_percent = MIN_BACKLIGHT_PERCENT) {
  // ledcSetup(BL_CH, BL_FREQ, BL_RES);
  // ledcAttach(BL_PIN, BL_CH);
  ledcAttach(BL_PIN, BL_FREQ, BL_RES);
  set_backlight_percent(start_percent);
}

// Initialize dynamic backlight control with a brightness of 75%
void initDynamicBacklightControl() {
  expander->pinMode(PCA_TFT_BACKLIGHT, INPUT);
  pinMode(BL_PIN, OUTPUT);
  backlight_begin(75);  // backlight brightness %
  Serial.printf("BL_PIN resolves to GPIO %d\n", (int)BL_PIN);
}

}