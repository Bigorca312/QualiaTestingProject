#pragma once
#include "Globals.h"

// Panel stuff:
#define PANEL_MEM_W 400
#define PANEL_VIS_W 320
#define PANEL_H     960

namespace DisplayInitialization {
  void initializeDisplay();
  void initializeLVGL();
  void lvgltesting();
  void startColorFade(uint32_t period_ms = 30, uint32_t margin_pct = 10);
  void DISPLAY_UPDATE_LOOP();
}
