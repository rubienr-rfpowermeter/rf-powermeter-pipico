#include "ui.h"

#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
  #include "lvgl.h"
#else
  #include "lvgl/lvgl.h"
#endif

#include "modules/lvgl/lv_input.h"
#include "modules/periphery/display/display_config.h"
#include "modules/periphery/input/input.h"
#include <cinttypes>
#include <string>

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

/*
void default_tab_view()
{
  lv_obj_t *tabview;
  tabview = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 50);

lv_obj_t *tab1 = lv_tabview_add_tab(tabview, "Meter");
lv_obj_t *tab2 = lv_tabview_add_tab(tabview, "Config");
lv_obj_t *tab3 = lv_tabview_add_tab(tabview, "Info");

lv_obj_t *label = lv_label_create(tab1);
lv_label_set_text(label, "First tab\n");

label = lv_label_create(tab2);
lv_label_set_text(label, "Second tab");


static lv_coord_t col_dsc[] = { 200, 240, LV_GRID_TEMPLATE_LAST };
static lv_coord_t row_dsc[] = { 400, LV_GRID_TEMPLATE_LAST };
lv_obj_set_style_grid_column_dsc_array(tab3, col_dsc, 0);
lv_obj_set_style_grid_row_dsc_array(tab3, row_dsc, 0);
// lv_obj_set_size(tab3, 480, 500);
lv_obj_center(tab3);
lv_obj_set_layout(tab3, LV_LAYOUT_GRID);

label                = lv_label_create(tab3);
char product_id[512] = { 0 };

const uint8_t rom_version = { rp2040_rom_version() };
snprintf(
  product_id, sizeof(product_id),
  "Firmware \n"
  " Version:      \t\t" PICO_PROGRAM_VERSION_STRING " \n"
  " Build Date:  \t" __DATE__ " \n"
  " Description:\t" PICO_PROGRAM_DESCRIPTION "\n"
  " Binary:     \t\t\t" PICO_PROGRAM_NAME " \n"
  " Build Type:  \t" PICO_CMAKE_BUILD_TYPE " \n"
  " pico-sdk V.: \t" PICO_SDK_VERSION_STRING " \n"
  " URL: " PICO_PROGRAM_URL "\n"
  "MCU\n"
  " Board:      \t\t\t" PICO_BOARD "\n"
  " Rom V.:      \t\t%" PRIu8 "\n",
  rom_version);

lv_label_set_text(label, product_id);
lv_obj_set_grid_cell(label, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

lv_obj_scroll_to_view_recursive(label, LV_ANIM_ON);


label = lv_label_create(tab3);
snprintf(
  product_id, sizeof(product_id),
  "Touch\n"
  " Product ID:  \t 0x%08" PRIx32 "\n"
  " Resolution:  \t %" PRIu16 " x %" PRIu16 "\n"
  " I2C Address: \t0x%" PRIx8 "\n"
  " Baud Rate:   \t %" PRIu32 "\n"
  " FW Version:  \t0x%08" PRIx16 "\n"
  " Vendor ID:     \t0x%02" PRIx8 "\n"
  "Display\n"
  " Resolution:    \t%" PRIu16 " x %" PRIu16 "\n"
  " Baud Rate:     \t%" PRIu32 "\n",
  globals.touch.product_id, globals.touch.width_px, globals.touch.height_px, globals.touch.i2c_address, globals.touch.baud_rate,
  globals.touch.firmware_version, globals.touch.vendor_id, globals.display.width_px, globals.display.height_px, globals.display.baud_rate);
lv_label_set_text(label, product_id);
lv_obj_set_grid_cell(label, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
}
*/


static void widgets_init(__unused UiData &data)
{
  lv_obj_t *screen = lv_obj_create(nullptr);
  lv_obj_clean(screen);

  // Create a Tab view object
  lv_obj_t *tabview;
  tabview = lv_tabview_create(screen, LV_DIR_RIGHT, 30);
  lv_group_add_obj(lv_input_get_keypad_group(), tabview);

  lv_obj_set_style_bg_color(tabview, lv_palette_lighten(LV_PALETTE_RED, 2), 0);

  lv_obj_t *tab_btns = lv_tabview_get_tab_btns(tabview);
  lv_obj_set_style_bg_color(tab_btns, lv_palette_darken(LV_PALETTE_GREY, 3), 0);
  lv_obj_set_style_text_color(tab_btns, lv_palette_lighten(LV_PALETTE_GREY, 5), 0);
  lv_obj_set_style_border_side(tab_btns, LV_BORDER_SIDE_RIGHT, (uint32_t)LV_PART_ITEMS | (uint32_t)LV_STATE_CHECKED);

  // Add 3 tabs (the tabs are page (lv_page) and can be scrolled
  lv_obj_t *tab1 = lv_tabview_add_tab(tabview, LV_SYMBOL_HOME);
  lv_obj_t *tab2 = lv_tabview_add_tab(tabview, LV_SYMBOL_PAUSE);
  lv_obj_t *tab3 = lv_tabview_add_tab(tabview, LV_SYMBOL_SETTINGS);
  lv_obj_t *tab4 = lv_tabview_add_tab(tabview, LV_SYMBOL_USB);


  lv_obj_set_style_bg_color(tab1, lv_palette_lighten(LV_PALETTE_AMBER, 3), 0);
  lv_obj_set_style_bg_opa(tab1, LV_OPA_COVER, 0);

  // Add content to the tabs
  lv_obj_t *label = lv_label_create(tab1);
  lv_label_set_text(label, "ft");

  label = lv_label_create(tab2);
  lv_label_set_text(label, "st");

  label = lv_label_create(tab3);
  lv_label_set_text(label, "tt");

  label = lv_label_create(tab4);
  lv_label_set_text(label, "ft");

  lv_obj_clear_flag(lv_tabview_get_content(tabview), LV_OBJ_FLAG_SCROLLABLE);

  lv_scr_load(screen);
}

void ui_init() { widgets_init(ui_data); }
