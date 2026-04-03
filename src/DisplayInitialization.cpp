#include "DisplayInitialization.h"

namespace DisplayInitialization {

// Display rotation stuff:
static constexpr lv_display_rotation_t kRotation = LV_DISPLAY_ROTATION_90; // <------- DEFINE ROTATION HERE
static constexpr int PAD_PIX = PANEL_MEM_W - PANEL_VIS_W;

int LEFT_PAD = 0, RIGHT_PAD = 0, TOP_PAD = 0, BOT_PAD = 0;
uint16_t screenWidth;
uint16_t screenHeight;
uint32_t bufSize_H_W;
uint32_t buffersize;
lv_display_t* disp;
lv_color_t* disp_draw_buf;
lv_color_t* disp_draw_buf2;

Arduino_ESP32RGBPanel* rgbpanel = nullptr;
Arduino_RGB_Display* gfx = nullptr;

#if DISPLAY_TESTING_MODE
  static lv_obj_t * s_fade_obj = nullptr;
  static void color_fade_cb(lv_timer_t * /*timer*/) {
    static uint16_t hue = 0;
    lv_color_t color = lv_color_hsv_to_rgb(hue, 100, 100);
    if (s_fade_obj) lv_obj_set_style_bg_color(s_fade_obj, color, 0);
    hue = (hue + 1) % 360;
  }

  // START COLOR FADE TEST
  // margin_pct: percentage of each axis to shave off each side (e.g. 10 = 10%)
  void startColorFade(uint32_t period_ms, uint32_t margin_pct) {
    lv_obj_t * screen = lv_screen_active();
    int32_t W = lv_obj_get_width(screen);
    int32_t H = lv_obj_get_height(screen);
    int32_t mx = (W * (int32_t)margin_pct) / 100;
    int32_t my = (H * (int32_t)margin_pct) / 100;
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_black(), 0);
  
    s_fade_obj = lv_obj_create(screen);
    lv_obj_set_size(s_fade_obj, W - mx * 2, H - my * 2);
    lv_obj_align(s_fade_obj, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_border_width(s_fade_obj, 0, 0);
    lv_obj_set_style_pad_all(s_fade_obj, 0, 0);
    lv_obj_set_style_radius(s_fade_obj, 0, 0);
  
    lv_timer_create(color_fade_cb, period_ms, nullptr);
  }

  // CREATE COLOR BORDER FOR LVGL SCREEN AREA
  void lvgltesting()
  {
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_black(), 0);
  
    lv_obj_t *screen = lv_screen_active();
    float W = lv_obj_get_width(screen) - 1;
    float H = lv_obj_get_height(screen) - 1;
    int32_t T = 6;
  
    static lv_point_precise_t top_pts[2];
    static lv_point_precise_t right_pts[2];
    static lv_point_precise_t bottom_pts[2];
    static lv_point_precise_t left_pts[2];
  
    top_pts[0]    = {0, 0}; top_pts[1]    = {W, 0};
    right_pts[0]  = {W, 0}; right_pts[1]  = {W, H};
    bottom_pts[0] = {0, H}; bottom_pts[1] = {W, H};
    left_pts[0]   = {0, 0}; left_pts[1]   = {0, H};
  
    lv_point_precise_t *pts_arr[] = { top_pts, right_pts, bottom_pts, left_pts };
    uint32_t colors[] = { 0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00 };
  
    static lv_style_t styles[4];
    for (int i = 0; i < 4; i++) {
        lv_obj_t *line = lv_line_create(screen);
        lv_line_set_points(line, pts_arr[i], 2);
        lv_style_init(&styles[i]);
        lv_style_set_line_width(&styles[i], T);
        lv_style_set_line_color(&styles[i], lv_color_hex(colors[i]));
        lv_style_set_line_rounded(&styles[i], false);
        lv_obj_add_style(line, &styles[i], 0);
    }
  }
#endif

// for HD458002C40 320x960
static void displaystartupvariables()
{
  //12000000 highest speed that works
  rgbpanel = new Arduino_ESP32RGBPanel(
    TFT_DE, TFT_VSYNC, TFT_HSYNC, TFT_PCLK, TFT_R1, TFT_R2, 
    TFT_R3, TFT_R4, TFT_R5, TFT_G0, TFT_G1, TFT_G2, TFT_G3,
    TFT_G4, TFT_G5, TFT_B1, TFT_B2, TFT_B3, TFT_B4, TFT_B5, 
    1 /* hync_polarity */, 50 /* hsync_front_porch */,
    10 /* hsync_pulse_width */, 30 /* hsync_back_porch */, 
    1 /* vsync_polarity */, 14 /* vsync_front_porch */,
    2 /* vsync_pulse_width */, 17 /* vsync_back_porch */, 
    1 /* pclk_active_neg */, 10000000 /* prefer_speed */,
    false /* useBigEndian */, 0 /* de_idle_high */, 0 /* pclk_idle_high */
    );

  // change auto-flush to TRUE to see more jitter/tearing
  gfx = new Arduino_RGB_Display(
    PANEL_VIS_W /* physical width */, PANEL_H /* physical height */, rgbpanel,
    kRotation /* rotation */, true /* auto_flush */, expander, GFX_NOT_DEFINED /* RST */,
    HD458002C40_init_operations, sizeof(HD458002C40_init_operations), LEFT_PAD /* col_offset */,
    TOP_PAD /* row_offset1 */, RIGHT_PAD /* col_offset2 */, BOT_PAD /* row_offset2 */
    );
}

static uint32_t LVGL_TICKWRAPPER(void) {
  return millis(); // Use Arduinos millis() as tick source
}

void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  // Serial.println("my_disp_flush called");
  // Serial.printf("area: x1=%d, y1=%d, x2=%d, y2=%d\n", area->x1, area->y1, area->x2, area->y2);

  #if DISPLAY_RENDER_MODE == 0
    // Partial mode: copy the dirty area to display
    gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)px_map, (area->x2 - area->x1 + 1), (area->y2 - area->y1 + 1));
    // gfx->flush();
  #endif

  #if DISPLAY_RENDER_MODE == 1
    // DIRECT MODE JUST, DOESNT WORK PROPERLY
    // gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)px_map, (area->x2 - area->x1 + 1), (area->y2 - area->y1 + 1));
    // gfx->flush();
  #endif

  #if DISPLAY_RENDER_MODE == 2
    gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)px_map, (area->x2 - area->x1 + 1), (area->y2 - area->y1 + 1));
    // gfx->flush();
  #endif

  // IS THIS NEEDED?????
  // if (lv_display_flush_is_last(disp)) {
  //   gfx->flush();    
  // }

  /*Call it to tell LVGL you are ready*/
  lv_display_flush_ready(disp);
}

void DISPLAY_UPDATE_LOOP() {
  #if DISPLAY_RENDER_MODE == 0
    // lv_display_flush_ready(disp); // ????
    // gfx->flush(); // ????
  #endif

  #if DISPLAY_RENDER_MODE == 1
    // i cannot get this to work.
    // lv_display_flush_ready(disp); // ????
    // gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)disp_draw_buf, screenWidth, screenHeight); // ????
    // gfx->flush(); // ????
  #endif

  #if DISPLAY_RENDER_MODE == 2
    gfx->flush();
  #endif
}

void initializeDisplay() {
  // Set up padding based on rotation
  // THIS IS A HACK. I DONT KNOW WHY I NEED TO DO THIS, BUT I DO.
  if (kRotation == LV_DISPLAY_ROTATION_0) {
    LEFT_PAD = PAD_PIX;
    RIGHT_PAD = 0;
    TOP_PAD = 0;
    BOT_PAD = 0;
  } else if (kRotation == LV_DISPLAY_ROTATION_90) {
    LEFT_PAD = 0;
    RIGHT_PAD = PAD_PIX;
    TOP_PAD = 0;
    BOT_PAD = 0;
  } else if (kRotation == LV_DISPLAY_ROTATION_180) {
    LEFT_PAD = 0;
    RIGHT_PAD = PAD_PIX;
    TOP_PAD = 0;
    BOT_PAD = 0;
  } else if (kRotation == LV_DISPLAY_ROTATION_270) { 
    // THIS STRAIGHT UP DOESNT WORK PROPERLY, I DONT KNOW WHY. THE IMAGE IS SHIFTED NO MATTER WHAT I DO. 
    // I THINK IT MIGHT BE A BUG IN THE GFX LIBRARY OR THE WAY IM USING IT.
    // I HAVE SET THEM ALL TO ZERO BECAUSE, I CANT GET IT TO WORK.
    LEFT_PAD = 0;
    RIGHT_PAD = 0;
    TOP_PAD = 0;
    BOT_PAD = 0;
  }

  displaystartupvariables(); //INIT THE DISPLAY VARIABLES (GFX AND STUFF)

  // Init Display
  if (!gfx->begin()) {
    Serial.println("gfx->begin() failed!");
  }

  screenWidth = gfx->width();
  screenHeight = gfx->height();
  gfx->fillScreen(0); // Clear screen to black
  
  switch (DISPLAY_RENDER_MODE)
  {
  case 0:
    bufSize_H_W = PANEL_MEM_W * 40;
    break;
  case 1:
    bufSize_H_W = PANEL_MEM_W * PANEL_H;
    break;  
  case 2:
    bufSize_H_W = PANEL_MEM_W * PANEL_H;
    break;
  default:
    bufSize_H_W = PANEL_MEM_W * PANEL_H;
    break;
  }
}

void initializeLVGL() {
  // Init LVGL
  lv_init();
  lv_tick_set_cb(LVGL_TICKWRAPPER);

  #if LV_USE_LOG != 0
    void my_print(lv_log_level_t level, const char* buf) {
      LV_UNUSED(level);
      Serial.println(buf);
      // Serial.flush();
    }
  #endif

  // disp = lv_display_create(PANEL_VIS_W, PANEL_H);
  disp = lv_display_create(screenWidth, screenHeight);
  buffersize = bufSize_H_W * lv_color_format_get_size(lv_display_get_color_format(disp));
  // lv_display_set_rotation(disp, kRotation);
  // lv_display_set_offset(disp, 0, 0);

  #if DISPLAY_RENDER_MODE == 0
    // Partial mode: small buffer for rendering strips
    disp_draw_buf = (lv_color_t*)heap_caps_malloc(buffersize, MALLOC_CAP_SPIRAM);
    lv_display_set_buffers(disp, disp_draw_buf, NULL, buffersize, LV_DISPLAY_RENDER_MODE_PARTIAL);
  #endif

  #if DISPLAY_RENDER_MODE == 1
    // THIS IS THE OLD STUFF:
    // disp_draw_buf = (lv_color_t*)gfx->getFramebuffer();
    // #if DOUBLE_BUFFERING
    //   disp_draw_buf2 = (lv_color_t*)heap_caps_malloc(buffersize, MALLOC_CAP_SPIRAM);
    //   lv_display_set_buffers(disp, disp_draw_buf, disp_draw_buf2, buffersize, LV_DISPLAY_RENDER_MODE_DIRECT);
    // #else
    //   lv_display_set_buffers(disp, disp_draw_buf, NULL, buffersize, LV_DISPLAY_RENDER_MODE_DIRECT);
    // #endif

    // THIS IS TO HELP ALIGN MEMORY, IT KINDA WORKS????
    lv_color_format_t cf = lv_display_get_color_format(disp);
    uint32_t bpp = lv_color_format_get_size(cf);
    uint32_t stride = PANEL_MEM_W * bpp;          // 400 * 2 = 800 bytes/row
    uint8_t* fb_base = (uint8_t*)gfx->getFramebuffer();

    // Point to the first visible pixel (skip LEFT_PAD columns)
    uint8_t* visible_start = fb_base + LEFT_PAD * bpp;

    static lv_draw_buf_t draw_buf1;
    lv_draw_buf_init(&draw_buf1, screenWidth, screenHeight, cf,
                     stride,           // <-- actual row stride in bytes
                     visible_start,
                     PANEL_H * stride);

    lv_display_set_draw_buffers(disp, &draw_buf1, NULL);
    lv_display_set_render_mode(disp, LV_DISPLAY_RENDER_MODE_DIRECT);
  #endif

  #if DISPLAY_RENDER_MODE == 2
    disp_draw_buf = (lv_color_t*)heap_caps_malloc(buffersize, MALLOC_CAP_SPIRAM);
    #if DOUBLE_BUFFERING
      disp_draw_buf2 = (lv_color_t*)heap_caps_malloc(buffersize, MALLOC_CAP_SPIRAM);
      lv_display_set_buffers(disp, disp_draw_buf, disp_draw_buf2, buffersize, LV_DISPLAY_RENDER_MODE_FULL);
    #else
      lv_display_set_buffers(disp, disp_draw_buf, NULL, buffersize, LV_DISPLAY_RENDER_MODE_FULL);
    #endif
  #endif

  lv_display_set_flush_cb(disp, my_disp_flush);
}

}  // namespace DisplayInitialization