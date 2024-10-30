#include "ui.h"

#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
  #include "lvgl.h"
#else
  #include "lvgl/lvgl.h"
#endif

#include "modules/globals/globals.h"
#include "modules/lvgl/lv_input.h"
#include "modules/periphery/display/display.h"
#include "modules/periphery/display/display_config.h"
#include <cinttypes>
#include <cstdio>
#include <hardware/watchdog.h>

typedef struct
{
  lv_obj_t *screen[4]; // screens
  lv_obj_t *cursor;    // cursor
  lv_obj_t *button;    // button
  lv_obj_t *label;     // label
  lv_obj_t *sw_1;      // switch 1
  lv_obj_t *sw_2;      // switch 2
  uint16_t click_num;  // number of button clicks

} UiData;

static UiData ui_data;

extern const lv_img_dsc_t LCD_1inch3;

[[noreturn]]
static void on_reboot_request(__unused lv_event_t *event)
{
  printf("main resetting, bye ...\n");
  watchdog_enable(1, false);
  while(true) { }
}


__unused static void on_key_pressed_on_info_tab(lv_event_t *event)
{
  printf("xxx event\n");
  // if(LV_KEY_DOWN == lv_indev_get_key(lv_indev_get_act())) { printf("vvv>\n"); }
  // else if(LV_KEY_UP == lv_indev_get_key(lv_indev_get_act())) { printf("^^^>\n"); }
}

static void init_info_tab(lv_obj_t *parent)
{
  lv_obj_center(parent);

  char product_info[512] = { 0 };

  // clang-format off
  snprintf(
    product_info, sizeof(product_info),
    PICO_PROGRAM_DESCRIPTION "\n"
    PICO_PROGRAM_URL "\n\n"
    "Version: " PICO_PROGRAM_VERSION_STRING "\n"
    "Build Date: " __DATE__ "\n"
    "Binary: " PICO_PROGRAM_NAME "\n"
    "Build type:" PICO_CMAKE_BUILD_TYPE "\n"
    "Pico-sdk V. " PICO_SDK_VERSION_STRING "\n\n"
    "Board: '" PICO_BOARD  "' ROM V.%" PRIu8 "\n\n"
    "Display: %" PRIu16 " x %" PRIu16 " baud %" PRIu32 "M\n"
    DISPLAY_NAME "\n",
    rp2040_rom_version(), DISPLAY_WIDTH_PX, DISPLAY_HEIGHT_PX, display_get_baud_rate() / 1000000);
  // clang-format on

  lv_obj_t *label = lv_label_create(parent);
  lv_label_set_text(label, product_info);

  lv_group_add_obj(lv_input_get_keypad_group(), parent);
  lv_obj_add_event_cb(parent, on_key_pressed_on_info_tab, LV_EVENT_KEY, nullptr);

  lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
}

static void widgets_init(__unused UiData &data)
{
  lv_obj_t *screen = lv_obj_create(nullptr);
  lv_obj_clean(screen);

  lv_obj_t *tabview;
  tabview = lv_tabview_create(screen, LV_DIR_RIGHT, 30);
  lv_group_add_obj(lv_input_get_keypad_group(), tabview);

  lv_obj_set_style_bg_color(tabview, lv_palette_lighten(LV_PALETTE_RED, 2), 0);

  lv_obj_t *tab_btns = lv_tabview_get_tab_btns(tabview);
  lv_obj_set_style_bg_color(tab_btns, lv_palette_darken(LV_PALETTE_GREY, 3), 0);
  lv_obj_set_style_text_color(tab_btns, lv_palette_lighten(LV_PALETTE_GREY, 5), 0);
  lv_obj_set_style_border_side(tab_btns, LV_BORDER_SIDE_RIGHT, (uint32_t)LV_PART_ITEMS | (uint32_t)LV_STATE_CHECKED);

  lv_obj_t *tab1 = lv_tabview_add_tab(tabview, LV_SYMBOL_HOME);
  lv_obj_t *tab2 = lv_tabview_add_tab(tabview, LV_SYMBOL_PAUSE);
  lv_obj_t *tab3 = lv_tabview_add_tab(tabview, LV_SYMBOL_SETTINGS);
  lv_obj_t *tab4 = lv_tabview_add_tab(tabview, LV_SYMBOL_USB);

  lv_obj_add_event_cb(tab_btns, on_reboot_request, LV_EVENT_LONG_PRESSED_REPEAT, nullptr);

  lv_obj_set_style_bg_color(tab1, lv_palette_lighten(LV_PALETTE_AMBER, 3), 0);
  lv_obj_set_style_bg_opa(tab1, LV_OPA_COVER, 0);

  lv_obj_t *label = lv_label_create(tab1);
  lv_label_set_text(label, "1st");

  label = lv_label_create(tab2);
  lv_label_set_text(label, "2nd");

  label = lv_label_create(tab3);
  lv_label_set_text(label, "3rd");

  lv_obj_clear_flag(lv_tabview_get_content(tabview), LV_OBJ_FLAG_SCROLLABLE);

  init_info_tab(tab4);

  lv_scr_load(screen);
}

void ui_init() { widgets_init(ui_data); }
