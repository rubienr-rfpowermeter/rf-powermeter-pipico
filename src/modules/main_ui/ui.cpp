#include "ui.h"

#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
  #include "lvgl.h"
#else
  #include "lvgl/lvgl.h"
#endif

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

static void widgets_init(UiData &data)
{
  static lv_style_t style_label;
  lv_style_init(&style_label);
  lv_style_set_text_font(&style_label, &lv_font_montserrat_16);

  // screen 1: a picture
  data.screen[0] = lv_obj_create(nullptr);

  lv_obj_t *image = lv_img_create(data.screen[0]);
  lv_img_set_src(image, &LCD_1inch3);
  lv_obj_align(image, LV_ALIGN_CENTER, 0, 0);

  lv_scr_load(data.screen[0]);

  // screen 2: User Interface 1
  data.screen[1] = lv_obj_create(nullptr);
  lv_obj_clear_flag(data.screen[1], LV_OBJ_FLAG_SCROLLABLE);

  // Create a 110x35 pixel button and add it to the center of the second screen
  data.button = lv_btn_create(data.screen[1]);
  lv_obj_set_size(data.button, 120, 40);
  lv_obj_align(data.button, LV_ALIGN_CENTER, 0, 0);

  // Create a label on the button and set its initial text to "Click:0"
  data.label = lv_label_create(data.button);
  lv_label_set_text(data.label, "Click:0");
  lv_obj_center(data.label);
  lv_obj_add_style(data.label, &style_label, 0);

  // Create an icon and add it to the second screen and set the image source of the icon to the GPS symbol
  data.cursor = lv_img_create(data.screen[1]);
  lv_img_set_src(data.cursor, LV_SYMBOL_GPS);

  // screen 3: User Interface 2
  data.screen[2] = lv_obj_create(nullptr);
  lv_obj_clear_flag(data.screen[2], LV_OBJ_FLAG_SCROLLABLE);

  // Create two switches and two labels in the third screen
  data.sw_1 = lv_switch_create(data.screen[2]);
  lv_obj_set_size(data.sw_1, 75, 40);
  lv_obj_align(data.sw_1, LV_ALIGN_RIGHT_MID, -40, -40);
  lv_obj_t *label_sw = lv_label_create(data.screen[2]);
  lv_obj_align(label_sw, LV_ALIGN_LEFT_MID, 40, -40);
  lv_label_set_text(label_sw, "KEY_X");

  // Add style to the label and set the font size to 16
  lv_obj_add_style(label_sw, &style_label, 0);

  data.sw_2 = lv_switch_create(data.screen[2]);
  lv_obj_set_size(data.sw_2, 75, 40);
  lv_obj_align(data.sw_2, LV_ALIGN_RIGHT_MID, -40, 40);
  label_sw = lv_label_create(data.screen[2]);
  lv_obj_align(label_sw, LV_ALIGN_LEFT_MID, 40, 40);
  lv_label_set_text(label_sw, "KEY_Y");
  lv_obj_add_style(label_sw, &style_label, 0);
}

void switch_to_next_screen(UiData &data)
{
  constexpr uint8_t screens_count = { sizeof(data.screen) / sizeof(lv_obj_t *) - 1 };

  const lv_obj_t *current_screen = { lv_scr_act() };
  for(uint8_t i = 0; i < screens_count; i++)
  {
    if(current_screen == data.screen[i])
    {
      const uint8_t next_screen_idx = { (uint8_t)((i + 1) % screens_count) };
      printf("next_screen=%" PRIu8 "\n", next_screen_idx);

      // interface switching animation is fade-in and fade-out, which lasts for 500 milliseconds,
      // which can effectively solve the jagged or ripple problems when switching interfaces
      lv_scr_load_anim(data.screen[next_screen_idx], LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, false);

      break;
    }
  }
}

static void widgets_up(lv_obj_t *widgets)
{
  int16_t y = lv_obj_get_y(widgets);
  y         = (y <= 0) ? 0 : y - 1;
  lv_obj_set_y(widgets, y);
}

static void widgets_down(lv_obj_t *widgets, uint16_t display_v_res_px)
{
  int16_t y       = lv_obj_get_y(widgets) + 1;
  const int16_t h = lv_obj_get_height(widgets);
  if(y > display_v_res_px - h) y = display_v_res_px - h;
  lv_obj_set_y(widgets, y);
}

static void widgets_left(lv_obj_t *widgets)
{
  int16_t x = lv_obj_get_x(widgets);
  x         = (x <= 0) ? 0 : x - 1;
  lv_obj_set_x(widgets, x);
}

static void widgets_right(lv_obj_t *widgets, uint16_t display_h_res_px)
{
  int16_t x       = lv_obj_get_x(widgets) + 1;
  const int16_t w = lv_obj_get_width(widgets);
  if(x > display_h_res_px - w) x = display_h_res_px - w;
  lv_obj_set_x(widgets, x);
}

static bool click_valid(lv_obj_t *cur, lv_obj_t *widgets)
{
  /*Get the coordinates and size of cursor*/
  lv_coord_t x1 = lv_obj_get_x(cur);
  lv_coord_t y1 = lv_obj_get_y(cur);
  lv_coord_t w1 = lv_obj_get_width(cur);
  // int h1 = lv_obj_get_height(cur);

  /*Get the coordinates and size of widgets*/
  lv_coord_t x2 = lv_obj_get_x(widgets);
  lv_coord_t y2 = lv_obj_get_y(widgets);
  lv_coord_t w2 = lv_obj_get_width(widgets);
  lv_coord_t h2 = lv_obj_get_height(widgets);

  /*Determine whether the right border of cursor is within the range of widget*/
  if(x1 + w1 >= x2 && x1 + w1 <= x2 + w2 && y1 >= y2 && y1 <= y2 + h2) { return true; }

  return false;
}

void handle_input_updates(const TrackedInputs &keys)
{
  // key b pressed event
  if(keys.b.is_pressed && keys.b.is_event)
  {
    switch_to_next_screen(ui_data); // Switch to the next screen
  }

  // if 2nd screen is active
  if(lv_scr_act() == ui_data.screen[1])
  {
    if(keys.up.is_pressed and !keys.up.is_event) // joystick up active
      widgets_up(ui_data.cursor);
    else if(keys.down.is_pressed and !keys.down.is_event) // joystick down active
      widgets_down(ui_data.cursor, DISPLAY_HEIGHT_PX);
    else if(keys.left.is_pressed and !keys.left.is_event) // joystick left active
      widgets_left(ui_data.cursor);
    else if(keys.right.is_pressed and !keys.right.is_event) // joystick right active
      widgets_right(ui_data.cursor, DISPLAY_WIDTH_PX);
    else if(keys.z.is_pressed and keys.z.is_event) // joystick z is active event
      lv_obj_set_pos(ui_data.cursor, 0, 0);


    if(keys.a.is_pressed and keys.a.is_event) // key a pressed event
    {
      // determine whether the click is valid
      if(click_valid(ui_data.cursor, ui_data.button))
      {
        char label_text[64];
        ui_data.click_num++; // Click count plus one
        sprintf(label_text, "Click:%d", ui_data.click_num);
        lv_label_set_text(ui_data.label, label_text);       // Update label
        lv_obj_add_state(ui_data.button, LV_STATE_PRESSED); // Set the button state to pressed
      }
    }
    else if(keys.a.is_released and keys.a.is_event)         // key a released event
      lv_obj_clear_state(ui_data.button, LV_STATE_PRESSED); // Set the button state to released
  }

  // if 3rd screen is active
  else if(lv_scr_act() == ui_data.screen[2])
  {
    if(keys.x.is_pressed and keys.x.is_event) // key x pressed event
    {
      if(lv_obj_has_state(ui_data.sw_1, LV_STATE_CHECKED))   // If SW_1 is currently selected
        lv_obj_clear_state(ui_data.sw_1, LV_STATE_CHECKED);  // Clear the selected state of SW_1
      else lv_obj_add_state(ui_data.sw_1, LV_STATE_CHECKED); // Set SW_1 to selected state
    }

    if(keys.y.is_pressed and keys.y.is_event) // key y pressed event
    {
      if(lv_obj_has_state(ui_data.sw_2, LV_STATE_CHECKED))   // If SW_2 is currently selected
        lv_obj_clear_state(ui_data.sw_2, LV_STATE_CHECKED);  // Clear the selected state of SW_2
      else lv_obj_add_state(ui_data.sw_2, LV_STATE_CHECKED); // Set SW_2 to selected state
    }
  }
}


/*
extern lv_img_dsc_t ai;
lv_obj_t *img1     = nullptr;
lv_obj_t *led1     = nullptr;
lv_obj_t *led2     = nullptr;
lv_obj_t *jy_label = nullptr;
*/

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

void ui_init()
{
  memset(ui_data.screen, 0, sizeof(ui_data.screen));
  ui_data.click_num = 0;

  widgets_init(ui_data);
}

static void print_info(const TrackedInputs &keys) { keys.print(); }

void ui_update_from_peripherals(const TrackedInputs &keys)
{
  if(keys.button_event || keys.joystick_event) print_info(keys);
  handle_input_updates(keys);
}
