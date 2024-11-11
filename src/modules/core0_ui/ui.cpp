#include "ui.h"

#include "lib/sample_data/TransactionData.h"
#include "lib/si/si_types.h"
#include "lvgl.h"
#include "modules/lvgl/lv_display.h"
#include "modules/lvgl/lv_input.h"
#include "modules/periphery/display/display.h"
#include "modules/periphery/display/display_config.h"
#include "modules/periphery/input/input.h"
#include <cinttypes>
#include <cstdio>
#include <hardware/watchdog.h>
#include <tuple>
#include <vector>

using si::SiFloat;

struct Tab0Page1
{
  lv_obj_t *tab { nullptr };

  lv_obj_t *avg_db_label { nullptr };
  lv_obj_t *avg_lin_label { nullptr };
};

struct Tab0Page0
{

  lv_obj_t *tab { nullptr };

  lv_obj_t *value_db_label { nullptr };
  lv_obj_t *avg_db_label { nullptr };
  lv_obj_t *min_db_label { nullptr };
  lv_obj_t *max_db_label { nullptr };

  lv_obj_t *value_lin_label { nullptr };
  lv_obj_t *avg_lin_label { nullptr };
  lv_obj_t *min_lin_label { nullptr };
  lv_obj_t *max_lin_label { nullptr };

  lv_obj_t *refresh_label { nullptr };
};

struct Tab0
{
  lv_obj_t *tab { nullptr };

  lv_obj_t *tab_view { nullptr };
  uint32_t  active_tab { UINT32_MAX };

  Tab0Page0 page0;
  Tab0Page1 page1;
};

struct Tab1
{
  lv_obj_t *tab { nullptr };

  lv_obj_t          *chart { nullptr };
  lv_obj_t          *current_value { nullptr };
  lv_chart_series_t *series { nullptr };
  lv_obj_t          *refresh_label { nullptr };
};

struct Tab2
{
  lv_obj_t *tab { nullptr };

  lv_obj_t *k0_label { nullptr };
  lv_obj_t *k1_label { nullptr };
  lv_obj_t *k2_label { nullptr };
  lv_obj_t *k3_label { nullptr };

  lv_obj_t *frequency_band_label { nullptr };
  lv_obj_t *probe_temp_label { nullptr };
};

struct Tab3
{
  lv_obj_t *tab { nullptr };
};

struct UiSamplingData
{
  TransactionData *sample { nullptr };
  uint32_t         previous_timestamp_us {};
} sampling;

struct UiWidgets
{
  char text_buffer[32] { 0 };

  lv_obj_t *screen { nullptr };
  lv_obj_t *tab_view { nullptr };
  uint32_t  active_tab { UINT32_MAX };

  struct
  {
    Tab0 tab0 {};
    Tab1 tab1 {};
    Tab2 tab2 {};
    Tab3 tab3 {};
  } tabs;
};

static UiWidgets      ui {};
static UiSamplingData ui_data {};

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

static void on_main_tab_changed_cb(__unused lv_event_t *event)
{
  printf("xxx main tab\n");
  ui.active_tab = lv_tabview_get_tab_active(ui.tab_view);
}

__unused static void on_tab0_tab_changed_cb(__unused lv_event_t *event)
{
  printf("xxx tab0\n");
  ui.tabs.tab0.active_tab = lv_tabview_get_tab_active(ui.tabs.tab0.tab_view);
}

__unused static void init_tab0_page0(Tab0Page0 &tab)
{
  lv_obj_t *parent { tab.tab };

  tab.avg_db_label   = lv_label_create(parent);
  tab.value_db_label = lv_label_create(parent);
  tab.min_db_label   = lv_label_create(parent);
  tab.max_db_label   = lv_label_create(parent);

  tab.avg_lin_label   = lv_label_create(parent);
  tab.value_lin_label = lv_label_create(parent);
  tab.min_lin_label   = lv_label_create(parent);
  tab.max_lin_label   = lv_label_create(parent);

  tab.refresh_label = lv_label_create(parent);

  const int32_t y_offset_px { 22 };
  const int32_t x_margin { 0 };
  const int32_t y_margin { 0 };

  const std::vector<std::tuple<lv_obj_t *, int32_t, int32_t>> todo {
    {    tab.avg_db_label, x_margin, y_margin + 0 * y_offset_px },
    {  tab.value_db_label, x_margin, y_margin + 1 * y_offset_px },
    {    tab.min_db_label, x_margin, y_margin + 2 * y_offset_px },
    {    tab.max_db_label, x_margin, y_margin + 3 * y_offset_px },
    {   tab.avg_lin_label, x_margin, y_margin + 4 * y_offset_px },
    { tab.value_lin_label, x_margin, y_margin + 5 * y_offset_px },
    {   tab.min_lin_label, x_margin, y_margin + 6 * y_offset_px },
    {   tab.max_lin_label, x_margin, y_margin + 7 * y_offset_px },
    {   tab.refresh_label, x_margin, y_margin + 8 * y_offset_px },
  };

  for (auto [label, x_px, y_px] : todo)
    lv_obj_set_pos(label, x_px, y_px);

  lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
}

__unused static void init_tab0_page1(Tab0Page1 &tab)
{
  lv_obj_t *parent { tab.tab };

  tab.avg_db_label  = lv_label_create(parent);
  tab.avg_lin_label = lv_label_create(parent);

  const int32_t y_offset_px { 22 };
  const int32_t x_margin { 0 };
  const int32_t y_margin { 0 };

  const std::vector<std::tuple<lv_obj_t *, int32_t, int32_t>> todo {
    {  tab.avg_db_label, x_margin, y_margin + 0 * y_offset_px },
    { tab.avg_lin_label, x_margin, y_margin + 1 * y_offset_px },
  };

  for (auto [label, x_px, y_px] : todo)
    lv_obj_set_pos(label, x_px, y_px);

  lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
}

static void init_tab0(__unused Tab0 &tab)
{
  tab.tab_view = lv_tabview_create(tab.tab);
  lv_tabview_set_tab_bar_position(tab.tab_view, LV_DIR_TOP);
  lv_obj_update_layout(tab.tab_view);
  lv_tabview_set_tab_bar_size(tab.tab_view, 30);
  lv_obj_set_size(tab.tab_view, lv_pct(60), lv_pct(60));
  lv_obj_set_style_bg_color(tab.tab_view, lv_palette_lighten(LV_PALETTE_PINK, 5), 0);

  lv_obj_t *tab_btns = lv_tabview_get_tab_btns(tab.tab_view);
  lv_obj_set_style_bg_color(tab_btns, lv_palette_darken(LV_PALETTE_PINK, 3), 0);
  lv_obj_set_style_text_color(tab_btns, lv_palette_lighten(LV_PALETTE_GREEN, 5), 0);
  lv_obj_set_style_border_side(tab_btns, LV_BORDER_SIDE_LEFT, (uint32_t)LV_PART_ITEMS | (uint32_t)LV_STATE_CHECKED);

  tab.page0.tab =  lv_tabview_add_tab(tab.tab_view, "tab1"); // todo rr - this breaks the ui as soon the tab becomes active
  // init_tab0_page0(tab.page0);

  // tab.page1.tab = lv_tabview_add_tab(tab.tab_view, "tab2");
  //init_tab0_page1(tab.page1);

  // lv_obj_t *tb { lv_tabview_get_tab_bar(tab.tab_view) };
  // lv_group_remove_obj(tb);
  // lv_group_add_obj(lv_input_get_keypad_group(), tb);

  // lv_obj_add_event_cb(tab.tab_view, on_tab0_tab_changed_cb, LV_EVENT_VALUE_CHANGED, nullptr);
  // tab.active_tab = 0;
  // lv_tabview_set_active(tab.tab_view, tab.active_tab, LV_ANIM_OFF);
}

static void init_tab1(Tab1 &tab)
{
  lv_obj_t *parent { tab.tab };

  tab.chart = lv_chart_create(parent);
  lv_chart_set_update_mode(tab.chart, LV_CHART_UPDATE_MODE_SHIFT);
  lv_obj_set_style_size(tab.chart, 0, 0, LV_PART_INDICATOR);
  lv_obj_set_size(tab.chart, 200, 150);
  lv_obj_center(tab.chart);

  lv_chart_set_point_count(tab.chart, 100);
  lv_chart_set_range(tab.chart, LV_CHART_AXIS_PRIMARY_Y, -60, 2);
  tab.series = lv_chart_add_series(tab.chart, lv_palette_main(LV_PALETTE_GREEN), LV_CHART_AXIS_PRIMARY_Y);

  uint32_t i;
  for (i = 0; i < 100; i++)
    lv_chart_set_next_value(tab.chart, tab.series, 0);

  lv_chart_refresh(tab.chart);

  tab.current_value = lv_label_create(parent);
  tab.refresh_label = lv_label_create(parent);
  lv_obj_set_pos(tab.current_value, 0, 0);
  lv_obj_set_pos(tab.refresh_label, 0, 200);

  lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
}

static void init_tab2(Tab2 &tab)
{
  lv_obj_t *parent { tab.tab };

  tab.k0_label = lv_label_create(parent);
  tab.k1_label = lv_label_create(parent);
  tab.k2_label = lv_label_create(parent);
  tab.k3_label = lv_label_create(parent);

  tab.frequency_band_label = lv_label_create(parent);
  tab.probe_temp_label     = lv_label_create(parent);

  const int32_t y_offset_px { 22 };
  const int32_t x_margin { 0 };
  const int32_t y_margin { 0 };

  const std::vector<std::tuple<lv_obj_t *, int32_t, int32_t>> todo {
    {             tab.k0_label, x_margin, y_margin + 0 * y_offset_px },
    {             tab.k1_label, x_margin, y_margin + 1 * y_offset_px },
    {             tab.k2_label, x_margin, y_margin + 2 * y_offset_px },
    {             tab.k3_label, x_margin, y_margin + 3 * y_offset_px },
    { tab.frequency_band_label, x_margin, y_margin + 4 * y_offset_px },
    {     tab.probe_temp_label, x_margin, y_margin + 5 * y_offset_px },
  };

  for (auto [label, x_px, y_px] : todo)
    lv_obj_set_pos(label, x_px, y_px);

  lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
}

static void init_tab3(Tab3 &tab)
{
  lv_obj_t *parent { tab.tab };
  lv_obj_center(parent);

  char product_info[512] { 0 };

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

  lv_obj_t *label { lv_label_create(parent) };
  lv_label_set_text(label, product_info);
  lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
  lv_obj_set_style_text_font(label, &lv_font_unscii_8, 0);
  lv_obj_update_layout(parent);
  lv_obj_set_size(label, lv_obj_get_width(parent) - 25, lv_obj_get_height(parent) - 10);

  lv_group_add_obj(lv_input_get_keypad_group(), parent);

  lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
}

static void init_widgets(UiWidgets &widgets)
{
  widgets.screen = lv_obj_create(nullptr);

  widgets.tab_view = lv_tabview_create(widgets.screen);
  lv_tabview_set_tab_bar_position(widgets.tab_view, LV_DIR_RIGHT);
  lv_tabview_set_tab_bar_size(widgets.tab_view, 30);
  lv_obj_set_style_pad_all(widgets.screen, 0, LV_PART_MAIN);
  lv_obj_set_size(widgets.tab_view, lv_pct(100), lv_pct(100));
  lv_obj_set_style_bg_color(widgets.tab_view, lv_palette_lighten(LV_PALETTE_PINK, 5), 0);

  lv_obj_t *tab_buttons { lv_tabview_get_tab_btns(widgets.tab_view) };
  lv_obj_set_style_bg_color(tab_buttons, lv_palette_darken(LV_PALETTE_PINK, 3), 0);
  lv_obj_set_style_text_color(tab_buttons, lv_palette_lighten(LV_PALETTE_PINK, 5), 0);
  lv_obj_set_style_border_side(tab_buttons, LV_BORDER_SIDE_RIGHT, (uint32_t)LV_PART_ITEMS | (uint32_t)LV_STATE_CHECKED);

  widgets.tabs.tab0.tab = lv_tabview_add_tab(widgets.tab_view, LV_SYMBOL_HOME);
  widgets.tabs.tab1.tab = lv_tabview_add_tab(widgets.tab_view, LV_SYMBOL_SHUFFLE);
  widgets.tabs.tab2.tab = lv_tabview_add_tab(widgets.tab_view, LV_SYMBOL_SETTINGS);
  widgets.tabs.tab3.tab = lv_tabview_add_tab(widgets.tab_view, LV_SYMBOL_USB);

  lv_obj_t *tb { lv_tabview_get_tab_bar(widgets.tab_view) };
  lv_group_remove_obj(tb);
  lv_group_add_obj(lv_input_get_buttons_group(), tb);

  lv_obj_t *btn3 { lv_obj_get_child(tb, 3) };
  lv_obj_add_event_cb(btn3, on_reboot_request_cb, LV_EVENT_LONG_PRESSED_REPEAT, nullptr);

  init_tab0(widgets.tabs.tab0);
  init_tab1(widgets.tabs.tab1);
  init_tab2(widgets.tabs.tab2);
  init_tab3(widgets.tabs.tab3);

  lv_obj_add_event_cb(widgets.tab_view, on_main_tab_changed_cb, LV_EVENT_VALUE_CHANGED, nullptr);
  widgets.active_tab = 1;
  lv_tabview_set_active(widgets.tab_view, widgets.active_tab, LV_ANIM_OFF);

  lv_obj_clear_flag(lv_tabview_get_content(widgets.tab_view), LV_OBJ_FLAG_SCROLLABLE);
  lv_scr_load(widgets.screen);
}

static void update_tab0_page0(Tab0Page0 &tab)
{
  const AveragedSiFloat &db_volt { ui_data.sample->converted_sample.value_dbv };
  const AveragedSiFloat &linv { ui_data.sample->converted_sample.value_linearv };

  const std::vector<std::tuple<const char *, const SiFloat &, lv_obj_t *>> todo {
    {   LV_SYMBOL_RIGHT " %6.1f %s%s%s", db_volt.value,  tab.value_db_label },
    {    LV_SYMBOL_DOWN " %6.1f %s%s%s",   db_volt.min,    tab.min_db_label },
    {      LV_SYMBOL_UP " %6.1f %s%s%s",   db_volt.max,    tab.max_db_label },
    { LV_SYMBOL_SHUFFLE " %6.1f %s%s%s",   db_volt.avg,    tab.avg_db_label },
    {   LV_SYMBOL_RIGHT " % 5.1f%s%s%s",    linv.value, tab.value_lin_label },
    {      LV_SYMBOL_DOWN "%5.1f%s%s%s",      linv.min,   tab.min_lin_label },
    {        LV_SYMBOL_UP "%5.1f%s%s%s",      linv.max,   tab.max_lin_label },
    {  LV_SYMBOL_SHUFFLE " %5.1f%s%s%s",      linv.avg,   tab.avg_lin_label }
  };

  for (auto &[format, value, label] : todo)
  {
    snprintf(
      ui.text_buffer, sizeof(ui.text_buffer), format, value.value, linearityToStr(value.lin), scaleToStr(value.scale),
      unitToStr(value.unit));
    lv_label_set_text(label, ui.text_buffer);
  }

  snprintf(ui.text_buffer, sizeof(ui.text_buffer), LV_SYMBOL_REFRESH " +%" PRIu32, (ui_data.sample->timestamp_us - ui_data.previous_timestamp_us) / 1000);
  lv_label_set_text(tab.refresh_label, ui.text_buffer);
}

static void update_tab0_page1(Tab0Page1 &tab)
{
  const AveragedSiFloat &db_volt { ui_data.sample->converted_sample.value_dbv };
  const AveragedSiFloat &linv { ui_data.sample->converted_sample.value_linearv };

  const std::vector<std::tuple<const char *, const SiFloat &, lv_obj_t *>> todo {
    { LV_SYMBOL_SHUFFLE " %6.1f %s%s%s", db_volt.avg,  tab.avg_db_label },
    {  LV_SYMBOL_SHUFFLE " %5.1f%s%s%s",    linv.avg, tab.avg_lin_label }
  };

  for (auto &[format, value, label] : todo)
  {
    snprintf(
      ui.text_buffer, sizeof(ui.text_buffer), format, value.value, linearityToStr(value.lin), scaleToStr(value.scale),
      unitToStr(value.unit));
    lv_label_set_text(label, ui.text_buffer);
  }
}

__unused static void update_tab0(Tab0 &tab)
{
  switch (tab.active_tab)
  {
  case 0:
    update_tab0_page0(tab.page0);
    return;
  case 1:
    update_tab0_page1(tab.page1);
    return;
  default:
    return;
  }
}

static void update_tab1(Tab1 tab)
{
  snprintf(ui.text_buffer, sizeof(ui.text_buffer), LV_SYMBOL_RIGHT " %5.1f", ui_data.sample->converted_sample.value_dbv.value.value);
  lv_label_set_text(tab.current_value, ui.text_buffer);

  lv_chart_set_next_value(tab.chart, tab.series, static_cast<int32_t>(ui_data.sample->converted_sample.value_dbv.value.value));
  const uint32_t p { lv_chart_get_point_count(tab.chart) };
  const uint32_t s { lv_chart_get_x_start_point(tab.chart, tab.series) };
  int32_t       *a { lv_chart_get_y_array(tab.chart, tab.series) };

  a[(s + 1) % p] = LV_CHART_POINT_NONE;
  a[(s + 2) % p] = LV_CHART_POINT_NONE;
  a[(s + 2) % p] = LV_CHART_POINT_NONE;

  lv_chart_refresh(tab.chart);

  snprintf(ui.text_buffer, sizeof(ui.text_buffer), LV_SYMBOL_REFRESH " +%" PRIu32, (ui_data.sample->timestamp_us - ui_data.previous_timestamp_us) / 1000);
  lv_label_set_text(tab.refresh_label, ui.text_buffer);
}

static void update_tab2(Tab2 &tab)
{

  const std::vector<std::tuple<const char *, const SiFloat &, lv_obj_t *>> todo {
    {  "k0: %6.1f %s%s%s", SiFloat { .value = ui_data.sample->correction_values.k0 },             tab.k0_label },
    {  "k1: %6.1f %s%s%s", SiFloat { .value = ui_data.sample->correction_values.k1 },             tab.k1_label },
    {  "k2: %6.1f %s%s%s", SiFloat { .value = ui_data.sample->correction_values.k2 },             tab.k2_label },
    {  "k3: %6.1f %s%s%s", SiFloat { .value = ui_data.sample->correction_values.k3 },             tab.k3_label },
    { "Band: %5.1f%s%s%s",         frequencyBandToSi(ui_data.sample->frequency_band), tab.frequency_band_label },
    { "Temp: %5.1f%s%s%s",             ui_data.sample->probe_temperature.celsius.avg,     tab.probe_temp_label },
  };

  for (auto &[format, value, label] : todo)
  {
    snprintf(
      ui.text_buffer, sizeof(ui.text_buffer), format, value.value, linearityToStr(value.lin), scaleToStr(value.scale),
      unitToStr(value.unit));
    lv_label_set_text(label, ui.text_buffer);
  }
}

static void update_tab3(__unused Tab3 tab) { }

void ui_init(TransactionData &sample)
{
  ui_data.sample = &sample;
  init_widgets(ui);
}

void ui_update()
{
  switch (ui.active_tab)
  {
  case 0:
    update_tab0(ui.tabs.tab0);
    ui_data.previous_timestamp_us = ui_data.sample->timestamp_us;
    return;
  case 1:
    update_tab1(ui.tabs.tab1);
    ui_data.previous_timestamp_us = ui_data.sample->timestamp_us;
    return;
  case 2:
    update_tab2(ui.tabs.tab2);
    return;
  case 3:
    update_tab3(ui.tabs.tab3);
    return;
  default:
    return;
  }
}
