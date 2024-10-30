#include "ui.h"

#include "lib/sample_data/TransactionBuffer.h"
#include "lvgl.h"
#include "modules/lvgl/lv_display.h"
#include "modules/lvgl/lv_input.h"
#include "modules/periphery/display/display.h"
#include "modules/periphery/display/display_config.h"
#include "modules/periphery/input/input.h"
#include <cinttypes>
#include <cstdio>
#include <hardware/watchdog.h>

struct UiData
{
  ResultUint16 *sample          = { nullptr };
  lv_obj_t     *value_label     = { nullptr };
  lv_obj_t     *avg_label       = { nullptr };
  lv_obj_t     *min_label       = { nullptr };
  lv_obj_t     *max_label       = { nullptr };
  char          text_buffer[32] = { 0 };
};

static UiData ui_data{};

extern const lv_img_dsc_t LCD_1inch3;

[[noreturn]]
static void on_reboot_request(__unused lv_event_t *event)
{
  printf("main resetting, bye ...\n");

  input_deinit();
  lv_input_deinit();
  lv_display_deinit();

  watchdog_enable(1, false);
  while (true) { }
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
  lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
  lv_obj_set_style_text_font(label, &lv_font_unscii_8, 0);
  lv_obj_update_layout(parent);
  lv_obj_set_size(label, lv_obj_get_width(parent) - 25, lv_obj_get_height(parent) - 10);

  lv_group_add_obj(lv_input_get_keypad_group(), parent);

  lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
}

static void widgets_init(UiData &data)
{
  lv_obj_t *screen = lv_obj_create(nullptr);
  lv_obj_clean(screen);

  lv_obj_t *tabview;
  tabview = lv_tabview_create(screen);
  lv_tabview_set_tab_bar_position(tabview, LV_DIR_RIGHT);
  lv_tabview_set_tab_bar_size(tabview, 30);

  lv_obj_set_style_bg_color(tabview, lv_palette_lighten(LV_PALETTE_PINK, 5), 0);

  lv_obj_t *tab_btns = lv_tabview_get_tab_btns(tabview);
  lv_obj_set_style_bg_color(tab_btns, lv_palette_darken(LV_PALETTE_PINK, 3), 0);
  lv_obj_set_style_text_color(tab_btns, lv_palette_lighten(LV_PALETTE_PINK, 5), 0);
  lv_obj_set_style_border_side(tab_btns, LV_BORDER_SIDE_NONE, (uint32_t)LV_PART_ITEMS | (uint32_t)LV_STATE_CHECKED);

  lv_obj_t *tab1 = lv_tabview_add_tab(tabview, LV_SYMBOL_HOME);
  lv_obj_t *tab2 = lv_tabview_add_tab(tabview, LV_SYMBOL_PAUSE);
  lv_obj_t *tab3 = lv_tabview_add_tab(tabview, LV_SYMBOL_SETTINGS);
  lv_obj_t *tab4 = lv_tabview_add_tab(tabview, LV_SYMBOL_USB);

  lv_obj_t *tb = lv_tabview_get_tab_bar(tabview);
  lv_group_add_obj(lv_input_get_buttons_group(), tb);
  lv_obj_t *btn3 = lv_obj_get_child(tb, 3);

  lv_obj_add_event_cb(btn3, on_reboot_request, LV_EVENT_LONG_PRESSED_REPEAT, nullptr);

  data.value_label = lv_label_create(tab1);
  data.avg_label   = lv_label_create(tab1);
  data.min_label   = lv_label_create(tab1);
  data.max_label   = lv_label_create(tab1);
  lv_obj_set_pos(data.value_label, 10, 10);
  lv_obj_set_pos(data.avg_label, 10, 25);
  lv_obj_set_pos(data.min_label, 10, 40);
  lv_obj_set_pos(data.max_label, 10, 55);
  lv_label_set_text(data.value_label, "value_label");
  lv_label_set_text(data.avg_label, "avg_label");
  lv_label_set_text(data.min_label, "min_label");
  lv_label_set_text(data.max_label, "max_label");

  lv_obj_t *label = lv_label_create(tab2);
  lv_label_set_text(label, "2nd");

  label = lv_label_create(tab3);
  lv_label_set_text(label, "3rd");

  init_info_tab(tab4);

  lv_obj_clear_flag(lv_tabview_get_content(tabview), LV_OBJ_FLAG_SCROLLABLE);
  lv_scr_load(screen);
}

void ui_init(ResultUint16 &sample)
{
  ui_data.sample = &sample;
  widgets_init(ui_data);
}

void ui_update()
{
  lv_snprintf(ui_data.text_buffer, sizeof(ui_data.text_buffer), "val=%" PRIu16 "|", ui_data.sample->value);
  lv_label_set_text(ui_data.value_label, ui_data.text_buffer);

  lv_snprintf(ui_data.text_buffer, sizeof(ui_data.text_buffer), "avg=%" PRIu16 "|", ui_data.sample->avg);
  lv_label_set_text(ui_data.avg_label, ui_data.text_buffer);

  lv_snprintf(ui_data.text_buffer, sizeof(ui_data.text_buffer), "min=%" PRIu16 "|", ui_data.sample->min);
  lv_label_set_text(ui_data.min_label, ui_data.text_buffer);

  lv_snprintf(ui_data.text_buffer, sizeof(ui_data.text_buffer), "max=%" PRIu16 "|", ui_data.sample->max);
  lv_label_set_text(ui_data.max_label, ui_data.text_buffer);
}
