#include "ui.h"

#include "lvgl.h"
#include "modules/lvgl/lv_display.h"
#include "modules/lvgl/lv_input.h"
#include "modules/periphery/display/display.h"
#include "modules/periphery/display/display_config.h"
#include "modules/periphery/input/input.h"
#include "modules/rf_power_meter/converter.h"
#include <cinttypes>
#include <cstdio>
#include <hardware/watchdog.h>

using namespace si;
using namespace rfpm;

struct UiData
{
  struct
  {
    TransactionData *sample{ nullptr };
    TransactionData  previous_sample{};

    SiUnit volt{ .value = 0, .lin = si::Linearity::Linear, .unit = si::Unit::Volt };
    SiUnit volt_avg{ .value = 0, .lin = si::Linearity::Linear, .unit = si::Unit::Volt };
    SiUnit volt_min{ .value = 0, .lin = si::Linearity::Linear, .unit = si::Unit::Volt };
    SiUnit volt_max{ .value = 0, .lin = si::Linearity::Linear, .unit = si::Unit::Volt };
    SiUnit volt_db{ .value = 0, .lin = si::Linearity::Db10, .unit = si::Unit::Volt };
    SiUnit volt_db_avg{ .value = 0, .lin = si::Linearity::Db10, .unit = si::Unit::Volt };
    SiUnit volt_db_min{ .value = 0, .lin = si::Linearity::Db10, .unit = si::Unit::Volt };
    SiUnit volt_db_max{ .value = 0, .lin = si::Linearity::Db10, .unit = si::Unit::Volt };
  } sampling;

  lv_obj_t *screen{ nullptr };

  struct
  {
    lv_obj_t *view{ nullptr };
    uint32_t  active_tab{ UINT32_MAX };

    struct
    {
      lv_obj_t *tab{ nullptr };
      lv_obj_t *value_label{ nullptr };
      lv_obj_t *avg_label{ nullptr };
      lv_obj_t *min_label{ nullptr };
      lv_obj_t *max_label{ nullptr };
      lv_obj_t *ts_label{ nullptr };
    } tab0;

    struct
    {
      lv_obj_t *tab{ nullptr };
      lv_obj_t *value_label{ nullptr };
      lv_obj_t *avg_label{ nullptr };
      lv_obj_t *min_label{ nullptr };
      lv_obj_t *max_label{ nullptr };
      lv_obj_t *ts_label{ nullptr };
    } tab1;

    struct
    {
      lv_obj_t *tab{ nullptr };
    } tab2;

    struct
    {
      lv_obj_t *tab{ nullptr };
    } tab3;
  } tab_view;

  char text_buffer[32]{ 0 };
};

static UiData ui_data{};

[[noreturn]]
static void on_reboot_request_cb(__unused lv_event_t *event)
{
  printf("main resetting, bye ...\n");

  input_deinit();
  lv_input_deinit();
  lv_display_deinit();

  watchdog_enable(1, false);
  while (true) { }
}

static void on_tab_changed_cb(__unused lv_event_t *event)
{
  ui_data.tab_view.active_tab = lv_tabview_get_tab_active(ui_data.tab_view.view);
}

static void init_info_tab(lv_obj_t *parent)
{
  lv_obj_center(parent);

  char product_info[512]{ 0 };

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

static void init_linear_watt_tab(lv_obj_t *parent)
{
  ui_data.tab_view.tab0.value_label = lv_label_create(parent);
  ui_data.tab_view.tab0.avg_label   = lv_label_create(parent);
  ui_data.tab_view.tab0.min_label   = lv_label_create(parent);
  ui_data.tab_view.tab0.max_label   = lv_label_create(parent);
  ui_data.tab_view.tab0.ts_label    = lv_label_create(parent);

  lv_obj_set_pos(ui_data.tab_view.tab0.value_label, 10, 10);
  lv_obj_set_pos(ui_data.tab_view.tab0.avg_label, 10, 25);
  lv_obj_set_pos(ui_data.tab_view.tab0.min_label, 10, 40);
  lv_obj_set_pos(ui_data.tab_view.tab0.max_label, 10, 55);
  lv_obj_set_pos(ui_data.tab_view.tab0.ts_label, 10, 70);

  lv_label_set_text(ui_data.tab_view.tab0.value_label, "??");
  lv_label_set_text(ui_data.tab_view.tab0.avg_label, "??");
  lv_label_set_text(ui_data.tab_view.tab0.min_label, "??");
  lv_label_set_text(ui_data.tab_view.tab0.max_label, "??");
  lv_label_set_text(ui_data.tab_view.tab0.ts_label, "??");

  lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
}

static void init_dbm_watt_tab(lv_obj_t *parent)
{
  ui_data.tab_view.tab1.value_label = lv_label_create(parent);
  ui_data.tab_view.tab1.avg_label   = lv_label_create(parent);
  ui_data.tab_view.tab1.min_label   = lv_label_create(parent);
  ui_data.tab_view.tab1.max_label   = lv_label_create(parent);
  ui_data.tab_view.tab1.ts_label    = lv_label_create(parent);

  lv_obj_set_pos(ui_data.tab_view.tab1.value_label, 10, 10);
  lv_obj_set_pos(ui_data.tab_view.tab1.avg_label, 10, 25);
  lv_obj_set_pos(ui_data.tab_view.tab1.min_label, 10, 40);
  lv_obj_set_pos(ui_data.tab_view.tab1.max_label, 10, 55);
  lv_obj_set_pos(ui_data.tab_view.tab1.ts_label, 10, 70);

  lv_label_set_text(ui_data.tab_view.tab1.value_label, "??");
  lv_label_set_text(ui_data.tab_view.tab1.avg_label, "??");
  lv_label_set_text(ui_data.tab_view.tab1.min_label, "??");
  lv_label_set_text(ui_data.tab_view.tab1.max_label, "??");
  lv_label_set_text(ui_data.tab_view.tab1.ts_label, "??");

  lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
}

static void init_config_tab(lv_obj_t *parent)
{
  lv_obj_t *label = lv_label_create(parent);
  lv_label_set_text(label, "config");

  lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
}

static void widgets_init()
{
  ui_data.screen = lv_obj_create(nullptr);
  lv_obj_clean(ui_data.screen);

  ui_data.tab_view.view = lv_tabview_create(ui_data.screen);
  lv_tabview_set_tab_bar_position(ui_data.tab_view.view, LV_DIR_RIGHT);
  lv_tabview_set_tab_bar_size(ui_data.tab_view.view, 30);

  lv_obj_set_style_bg_color(ui_data.tab_view.view, lv_palette_lighten(LV_PALETTE_PINK, 5), 0);

  lv_obj_t *tab_btns = lv_tabview_get_tab_btns(ui_data.tab_view.view);
  lv_obj_set_style_bg_color(tab_btns, lv_palette_darken(LV_PALETTE_PINK, 3), 0);
  lv_obj_set_style_text_color(tab_btns, lv_palette_lighten(LV_PALETTE_PINK, 5), 0);
  lv_obj_set_style_border_side(tab_btns, LV_BORDER_SIDE_NONE, (uint32_t)LV_PART_ITEMS | (uint32_t)LV_STATE_CHECKED);

  ui_data.tab_view.tab0.tab = lv_tabview_add_tab(ui_data.tab_view.view, LV_SYMBOL_HOME);
  ui_data.tab_view.tab1.tab = lv_tabview_add_tab(ui_data.tab_view.view, LV_SYMBOL_PAUSE);
  ui_data.tab_view.tab2.tab = lv_tabview_add_tab(ui_data.tab_view.view, LV_SYMBOL_SETTINGS);
  ui_data.tab_view.tab3.tab = lv_tabview_add_tab(ui_data.tab_view.view, LV_SYMBOL_USB);

  lv_obj_t *tb = lv_tabview_get_tab_bar(ui_data.tab_view.view);
  lv_group_add_obj(lv_input_get_buttons_group(), tb);
  lv_obj_t *btn3 = lv_obj_get_child(tb, 3);

  lv_obj_add_event_cb(btn3, on_reboot_request_cb, LV_EVENT_LONG_PRESSED_REPEAT, nullptr);

  init_linear_watt_tab(ui_data.tab_view.tab0.tab);
  init_dbm_watt_tab(ui_data.tab_view.tab1.tab);
  init_config_tab(ui_data.tab_view.tab2.tab);
  init_info_tab(ui_data.tab_view.tab3.tab);

  lv_obj_add_event_cb(ui_data.tab_view.view, on_tab_changed_cb, LV_EVENT_VALUE_CHANGED, nullptr);
  ui_data.tab_view.active_tab = 0;
  lv_tabview_set_active(ui_data.tab_view.view, ui_data.tab_view.active_tab, LV_ANIM_OFF);

  lv_obj_clear_flag(lv_tabview_get_content(ui_data.tab_view.view), LV_OBJ_FLAG_SCROLLABLE);
  lv_scr_load(ui_data.screen);
}

static void ui_update_dbm_watt_tab()
{
  snprintf(ui_data.text_buffer, sizeof(ui_data.text_buffer), "val=%4" PRIu16 " w", ui_data.sampling.sample->raw_sample.value);
  lv_label_set_text(ui_data.tab_view.tab1.value_label, ui_data.text_buffer);

  snprintf(ui_data.text_buffer, sizeof(ui_data.text_buffer), "min=%4" PRIu16 " w", ui_data.sampling.sample->raw_sample.min);
  lv_label_set_text(ui_data.tab_view.tab1.avg_label, ui_data.text_buffer);

  snprintf(ui_data.text_buffer, sizeof(ui_data.text_buffer), "max=%4" PRIu16 " w", ui_data.sampling.sample->raw_sample.max);
  lv_label_set_text(ui_data.tab_view.tab1.min_label, ui_data.text_buffer);

  snprintf(ui_data.text_buffer, sizeof(ui_data.text_buffer), "avg=%4" PRIu16 " w", ui_data.sampling.sample->raw_sample.avg);
  lv_label_set_text(ui_data.tab_view.tab1.max_label, ui_data.text_buffer);

  snprintf(
    ui_data.text_buffer, sizeof(ui_data.text_buffer), "ts_ms=+%" PRIu32,
    ui_data.sampling.sample->timestamp_ms - ui_data.sampling.previous_sample.timestamp_ms);
  lv_label_set_text(ui_data.tab_view.tab1.ts_label, ui_data.text_buffer);
}

#include <list>
#include <tuple>

static void convert_sample()
{
  static const std::list<std::tuple<SiUnit &, SiUnit &, uint16_t &>> todo{
    { ui_data.sampling.volt_avg, ui_data.sampling.volt_db_avg, ui_data.sampling.sample->raw_sample.avg },
    { ui_data.sampling.volt_min, ui_data.sampling.volt_db_min, ui_data.sampling.sample->raw_sample.min },
    { ui_data.sampling.volt_max, ui_data.sampling.volt_db_max, ui_data.sampling.sample->raw_sample.max }
  };

  for (auto [out_linear, out_db, in] : todo)
  {
    const auto convertedDbv{ dbvCorrectedFromDbv(dbvFromAdc(voltFrom12bitAdc(in, AD7887_V_REF), FrequencyBand::GHz_0_9), 0, { 1, 0, 0 }) };
    out_db.value = convertedDbv;
    const SiUnit linearDbv{ toLinearV(convertedDbv) };
    out_linear.scale = linearDbv.scale;
    out_linear.value = linearDbv.value;
  }
}

static void ui_update_linear_watt_tab()
{
  convert_sample();

  static const std::list<std::tuple<const char *, lv_obj_t *, SiUnit &>> todo{
    {              "->% 6.2lf%s%s", ui_data.tab_view.tab0.value_label,        ui_data.sampling.volt },
    { LV_SYMBOL_DOWN " % 6.2lf%s%s",   ui_data.tab_view.tab0.avg_label,    ui_data.sampling.volt_avg },
    {   LV_SYMBOL_UP " % 6.2lf%s%s",   ui_data.tab_view.tab0.min_label,     ui_data.sampling.volt_db },
    {               "~ % 6.2lf%s%s",   ui_data.tab_view.tab0.max_label, ui_data.sampling.volt_db_avg },
  };

  for (auto [format, out, in] : todo)
  {
    snprintf(ui_data.text_buffer, sizeof(ui_data.text_buffer), format, in.value, scaleToStr(in.scale), unitToStr(in.unit));
    lv_label_set_text(out, ui_data.text_buffer);
  }

  snprintf(
    ui_data.text_buffer, sizeof(ui_data.text_buffer), LV_SYMBOL_REFRESH " +%" PRIu32,
    ui_data.sampling.sample->timestamp_ms - ui_data.sampling.previous_sample.timestamp_ms);
  lv_label_set_text(ui_data.tab_view.tab0.ts_label, ui_data.text_buffer);
}

void ui_init(TransactionData &sample)
{
  ui_data.sampling.sample = &sample;
  widgets_init();
}

void ui_update()
{
  switch (ui_data.tab_view.active_tab)
  {
  case 0:
    ui_update_linear_watt_tab();
    ui_data.sampling.previous_sample = *ui_data.sampling.sample;
    return;
  case 1:
    ui_update_dbm_watt_tab();
    ui_data.sampling.previous_sample = *ui_data.sampling.sample;
    return;
  case 2:
  case 3:
  default:
    return;
  }
}
