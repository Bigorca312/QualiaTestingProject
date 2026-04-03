#include "Globals.h"
#include "DisplayInitialization.h"
#include "BacklightControl.h"

// for qualia expander
Arduino_XCA9554SWSPI* expander =  new Arduino_XCA9554SWSPI(PCA_TFT_RESET, PCA_TFT_CS, PCA_TFT_SCK, PCA_TFT_MOSI, &Wire, 0x3F);

void setup() {
  Serial.begin(115200);
  delay(1000); // Wait for Serial to be ready
  Wire.setClock(1000000); // is this needed??????

  #if ENABLE_PWM_BACKLIGHT
    // PWM backlight control only works if the jumper is soldered on the qualia board
    BacklightControl::initDynamicBacklightControl();
    BacklightControl::set_backlight_percent(50);
  #else
    expander->pinMode(PCA_TFT_BACKLIGHT, OUTPUT);
    expander->digitalWrite(PCA_TFT_BACKLIGHT, HIGH);  // backlight on
  #endif

  // Initialize display and LVGL
  DisplayInitialization::initializeDisplay(); // Initialize display (GFX)
  DisplayInitialization::initializeLVGL();    // LVGL

  DisplayInitialization::startColorFade(30, 20);

  Serial.println("Setup done");
}

void loop() {
  // put your main code here, to run repeatedly:
  lv_timer_handler(); /* let the GUI do its work */

  DisplayInitialization::DISPLAY_UPDATE_LOOP(); // MUST BE HERE

  // delay(10); // internally this calls vTaskDelay? is this needed?
}