#include "ui.h"

#include "modules/periphery/display/display_config.h"
#include "modules/periphery/input/buttons.h"
#include "modules/periphery/input/joystick.h"
#include <cinttypes>
#include <string>

enum Update : uint8_t
{
  Update_PRESSED_NO_UPDATE = 0,
  Update_RELEASED_NO_UPDATE,
  Update_PRESS_EVENT,
  Update_RELEASE_EVENT,
  Update_NO_UPDATE
};

constexpr const char *
  update_to_str[] = { [Update_PRESSED_NO_UPDATE] = "1 ", [Update_RELEASED_NO_UPDATE] = "0 ", [Update_PRESS_EVENT] = "1+", [Update_RELEASE_EVENT] = "0+", [Update_NO_UPDATE] = "??" };

typedef struct
{
  uint8_t a;
  uint8_t b;
  uint8_t x;
  uint8_t y;
} ButtonsHistory;

typedef struct
{
  enum Update a;
  enum Update b;
  enum Update x;
  enum Update y;
} ButtonsState;

typedef struct
{
  uint8_t right;
  uint8_t left;
  uint8_t up;
  uint8_t down;
  uint8_t z;
} JoystickHistory;

typedef struct
{
  enum Update right;
  enum Update left;
  enum Update up;
  enum Update down;
  enum Update z;
} JoystickState;

typedef struct
{
  lv_obj_t *screen[4]; // Used to store different screens
  lv_obj_t *cursor;    // Cursor
  lv_obj_t *button;    // Button
  lv_obj_t *label;     // Label
  lv_obj_t *sw_1;      // Switch 1
  lv_obj_t *sw_2;      // Switch 2
  uint16_t click_num;  // Number of button clicks

  ButtonsHistory buttons_previous;
  ButtonsHistory buttons_current;
  JoystickHistory joystick_current;
  JoystickHistory joystick_previous;

  ButtonsState buttons;
  JoystickState joystick;
} UiData;

static UiData ui_data = { 0 };

static void advance_counter(uint8_t &counter, bool plus)
{
  if(plus && (counter < 5)) counter++;
  else if(!plus && counter > 0) counter--;
}

static enum Update compute_update(uint8_t prev_counter, uint8_t curr_counter)
{
  constexpr uint8_t threshold = { 3 };

  if(prev_counter > threshold and curr_counter <= threshold) return Update_RELEASE_EVENT;
  else if(prev_counter < threshold and curr_counter >= threshold) return Update_PRESS_EVENT;
  else if(prev_counter < threshold and curr_counter < threshold) return Update_RELEASED_NO_UPDATE;
  else if(prev_counter > threshold and curr_counter > threshold) return Update_PRESSED_NO_UPDATE;
  else return Update_NO_UPDATE;
}

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
  int y = lv_obj_get_y(widgets); // Get the y coordinate of widgets
  y -= 1;                        // The y coordinate is minus 1
  if(y < 0) y = 0;               // Limit the y coordinate of widgets so that they do not exceed the top of the screen
  lv_obj_set_y(widgets, y);      // Set the y coordinate of widgets
}

static void widgets_down(lv_obj_t *widgets, uint16_t DISP_VER_RES)
{
  int y = lv_obj_get_y(widgets);      // Get the y coordinate of widgets
  int h = lv_obj_get_height(widgets); // Get the height of widgets
  y += 1;                             // Add 1 to the y coordinate
  if(y > LCD_HEIGHT_PX - h)
    y = LCD_HEIGHT_PX - h;  // Control the y coordinate of widgets so that their bottom cannot exceed the bottom of the screen
  lv_obj_set_y(widgets, y); // Set the y coordinate of widgets
}

static void widgets_left(lv_obj_t *widgets)
{
  int x = lv_obj_get_x(widgets); // Get the x coordinate of widgets
  x -= 1;                        // The x coordinate is minus 1
  if(x < 0) x = 0;               // Limit the x coordinate of widgets so that they do not exceed the left of the screen
  lv_obj_set_x(widgets, x);      // Set the x coordinate of widgets
}

static void widgets_right(lv_obj_t *widgets, uint16_t DISP_HOR_RES)
{
  int x = lv_obj_get_x(widgets);     // Get the x coordinate of widgets
  int w = lv_obj_get_width(widgets); // Get the width of widgets
  x += 1;                            // Add 1 to the x coordinate
  if(x > LCD_WIDTH_PX - w)
    x = LCD_WIDTH_PX - w; // Control the x coordinate of widgets so that their right side cannot exceed the right side of the screen
  lv_obj_set_x(widgets, x); // Set the y coordinate of widgets
}

static bool click_valid(lv_obj_t *cur, lv_obj_t *widgets)
{
  /*Get the coordinates and size of cursor*/
  int x1 = lv_obj_get_x(cur);
  int y1 = lv_obj_get_y(cur);
  int w1 = lv_obj_get_width(cur);
  // int h1 = lv_obj_get_height(cur);

  /*Get the coordinates and size of widgets*/
  int x2 = lv_obj_get_x(widgets);
  int y2 = lv_obj_get_y(widgets);
  int w2 = lv_obj_get_width(widgets);
  int h2 = lv_obj_get_height(widgets);

  /*Determine whether the right border of cursor is within the range of widget*/
  if(x1 + w1 >= x2 && x1 + w1 <= x2 + w2 && y1 >= y2 && y1 <= y2 + h2) { return true; }

  return false;
}

void handle_input_updates()
{
  /*If KEY_B is currently pressed and its previous state is released*/
  if(Update_PRESS_EVENT == ui_data.buttons.b)
  {
    switch_to_next_screen(ui_data); // Switch to the next screen
  }

  /*If the active screen is the second*/
  if(lv_scr_act() == ui_data.screen[1])
  {
    if(Update_PRESSED_NO_UPDATE == ui_data.joystick.up) // Joystick up
    {
      widgets_up(ui_data.cursor); // Move the pointer up
    }
    else if(Update_PRESSED_NO_UPDATE == ui_data.joystick.down) // Joystick down
    {
      widgets_down(ui_data.cursor, LCD_HEIGHT_PX); // Move the pointer down
    }
    else if(Update_PRESSED_NO_UPDATE == ui_data.joystick.left) // Joystick left
    {
      widgets_left(ui_data.cursor); // Move the pointer left
    }
    else if(Update_PRESSED_NO_UPDATE == ui_data.joystick.right) // Joystick right
    {
      widgets_right(ui_data.cursor, LCD_WIDTH_PX); // Move the pointer right
    }
    else if(Update_PRESS_EVENT == ui_data.joystick.z) // Joystick press down
    {
      lv_obj_set_pos(ui_data.cursor, 0, 0); // Set the pointer coordinates to 0, 0
    }

    /*If KEY_A is currently pressed and its previous state is released*/
    if(Update_PRESS_EVENT == ui_data.buttons.a)
    {
      /*Determine whether the click is valid*/
      if(click_valid(ui_data.cursor, ui_data.button))
      {
        char label_text[64];
        ui_data.click_num++; // Click count plus one
        sprintf(label_text, "Click:%d", ui_data.click_num);
        lv_label_set_text(ui_data.label, label_text);       // Update label
        lv_obj_add_state(ui_data.button, LV_STATE_PRESSED); // Set the button state to pressed
      }
    }
    /*If KEY_A is currently released*/
    else if(Update_RELEASE_EVENT == ui_data.buttons.a)
    {
      lv_obj_clear_state(ui_data.button, LV_STATE_PRESSED); // Set the button state to released
    }
  }

  /*If the active screen is the third*/
  else if(lv_scr_act() == ui_data.screen[2])
  {
    /*If KEY_X is currently pressed and its previous state is released*/
    if(Update_RELEASE_EVENT == ui_data.buttons.x)
    {
      if(lv_obj_has_state(ui_data.sw_1, LV_STATE_CHECKED))   // If SW_1 is currently selected
        lv_obj_clear_state(ui_data.sw_1, LV_STATE_CHECKED);  // Clear the selected state of SW_1
      else lv_obj_add_state(ui_data.sw_1, LV_STATE_CHECKED); // Set SW_1 to selected state
    }

    /*If KEY_Y is currently pressed and its previous state is released*/
    if(Update_RELEASE_EVENT == ui_data.buttons.y)
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

static void print_info()
{
  /*
  printf(
    "a=%" PRIu8 "/%" PRIu8 " b=%" PRIu8 "/%" PRIu8 " x=%" PRIu8 "/%" PRIu8 " y=%" PRIu8 "/%" PRIu8 "\n",
    ui_data.buttons_previous.a, ui_data.buttons_current.a, ui_data.buttons_previous.b, ui_data.buttons_current.b,
    ui_data.buttons_previous.x, ui_data.buttons_current.x, ui_data.buttons_previous.y, ui_data.buttons_current.y);
    */
  printf(
    "a=%s b=%s x=%s y=%s | ", update_to_str[ui_data.buttons.a], update_to_str[ui_data.buttons.b],
    update_to_str[ui_data.buttons.x], update_to_str[ui_data.buttons.y]);

  printf(
    "u=%s d=%s l=%s r=%s z=%s\n", update_to_str[ui_data.joystick.up], update_to_str[ui_data.joystick.down],
    update_to_str[ui_data.joystick.left], update_to_str[ui_data.joystick.right], update_to_str[ui_data.joystick.z]);
}

void ui_update_from_peripherals()
{
  { // track button updates
    const uint8_t mask = { buttons_get_mask() };
    advance_counter(ui_data.buttons_current.a, buttons_any_pressed_in_mask(BUTTONS_MASK_A, mask));
    advance_counter(ui_data.buttons_current.b, buttons_any_pressed_in_mask(BUTTONS_MASK_B, mask));
    advance_counter(ui_data.buttons_current.x, buttons_any_pressed_in_mask(BUTTONS_MASK_X, mask));
    advance_counter(ui_data.buttons_current.y, buttons_any_pressed_in_mask(BUTTONS_MASK_Y, mask));

    ui_data.buttons.a = compute_update(ui_data.buttons_previous.a, ui_data.buttons_current.a);
    ui_data.buttons.b = compute_update(ui_data.buttons_previous.b, ui_data.buttons_current.b);
    ui_data.buttons.x = compute_update(ui_data.buttons_previous.x, ui_data.buttons_current.x);
    ui_data.buttons.y = compute_update(ui_data.buttons_previous.y, ui_data.buttons_current.y);
  }

  { // track joystick updates
    const uint8_t mask = { joystick_get_mask() };
    advance_counter(ui_data.joystick_current.up, joystick_any_active_in_mask(JOYSTICK_MASK_UP, mask));
    advance_counter(ui_data.joystick_current.down, joystick_any_active_in_mask(JOYSTICK_MASK_DOWN, mask));
    advance_counter(ui_data.joystick_current.left, joystick_any_active_in_mask(JOYSTICK_MASK_LEFT, mask));
    advance_counter(ui_data.joystick_current.right, joystick_any_active_in_mask(JOYSTICK_MASK_RIGHT, mask));
    advance_counter(ui_data.joystick_current.z, joystick_any_active_in_mask(JOYSTICK_MASK_Z, mask));

    ui_data.joystick.up    = compute_update(ui_data.joystick_previous.up, ui_data.joystick_current.up);
    ui_data.joystick.down  = compute_update(ui_data.joystick_previous.down, ui_data.joystick_current.down);
    ui_data.joystick.left  = compute_update(ui_data.joystick_previous.left, ui_data.joystick_current.left);
    ui_data.joystick.right = compute_update(ui_data.joystick_previous.right, ui_data.joystick_current.right);
    ui_data.joystick.z     = compute_update(ui_data.joystick_previous.z, ui_data.joystick_current.z);
  }

  if(
    Update_RELEASE_EVENT == ui_data.buttons.a || Update_PRESS_EVENT == ui_data.buttons.a ||
    Update_RELEASE_EVENT == ui_data.buttons.b || Update_PRESS_EVENT == ui_data.buttons.b ||
    Update_RELEASE_EVENT == ui_data.buttons.x || Update_PRESS_EVENT == ui_data.buttons.x ||
    Update_RELEASE_EVENT == ui_data.buttons.y || Update_PRESS_EVENT == ui_data.buttons.y ||
    Update_RELEASE_EVENT == ui_data.joystick.up || Update_PRESS_EVENT == ui_data.joystick.up ||
    Update_RELEASE_EVENT == ui_data.joystick.down || Update_PRESS_EVENT == ui_data.joystick.down ||
    Update_RELEASE_EVENT == ui_data.joystick.left || Update_PRESS_EVENT == ui_data.joystick.left ||
    Update_RELEASE_EVENT == ui_data.joystick.right || Update_PRESS_EVENT == ui_data.joystick.right ||
    Update_RELEASE_EVENT == ui_data.joystick.z || Update_PRESS_EVENT == ui_data.joystick.z)
  {
    print_info();
    handle_input_updates();
  }

  ui_data.buttons_previous  = ui_data.buttons_current;
  ui_data.joystick_previous = ui_data.joystick_current;
}
