#include "modules/lvgl/lv_port_disp.h"
#include "modules/lvgl/lv_port_indev.h"
#include "modules/periphery/buttons.h"
#include "modules/periphery/buzzer.h"
#include "modules/periphery/joystick.h"
#include "modules/periphery/leds.h"
#include "modules/periphery/ws2812/rgbw.h"

#include "demos/keypad_encoder/lv_demo_keypad_encoder.h"
#include "lvgl.h"
#include "modules/globals/globals.h"
#include "pico/stdlib.h"

#include <cstdio>

extern lv_img_dsc_t ai;
lv_obj_t *img1 = nullptr;
lv_obj_t *led1 = nullptr;
lv_obj_t *led2 = nullptr;
lv_obj_t *jy_label = nullptr;

volatile uint32_t systemTicksMs = {0};
bool ms_tick_timer_cb(struct repeating_timer *__unused t)
{
    systemTicksMs += 1;
    lv_tick_inc(1);
    return true;
}

static void handle_buzzer(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_VALUE_CHANGED) { buzzer_beep(); }
}

static void handle_keypad(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED)
    {
        lv_obj_del(img1);
        lv_obj_clean(lv_scr_act());
        busy_wait_ms(100);

        lv_demo_keypad_encoder();
    }
}

static void handle_clr_rgb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) { rgbw_put_pixel(rgbw_to_u32(0, 0, 0)); }
}

static void slider_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if(code == LV_EVENT_VALUE_CHANGED)
    {
        lv_color_t color = lv_colorwheel_get_rgb(obj);
        rgbw_put_pixel(rgbw_to_u32(color.ch.red << 5, ((color.ch.green_h << 2) + color.ch.green_h) << 2, (color.ch.blue << 3)));
    }
}

void gpio_buttons_callback(uint gpio, uint32_t __unused events)
{
    switch(gpio)
    {
    case buttons_gpio_1:
        lv_led_toggle(led1);
        gpio_xor_mask(1ul << 16);
        break;
    case buttons_gpio_2:
        lv_led_toggle(led2);
        gpio_xor_mask(1ul << 17);
        break;
    default:
        break;
    }
}


static void handle_hw(lv_event_t *e)
{
    lv_obj_t *label;

    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED)
    {
        lv_obj_del(img1);
        lv_obj_clean(lv_scr_act());
        busy_wait_ms(100);

        buzzer_init();

        lv_obj_t *beep_btn = lv_btn_create(lv_scr_act());
        lv_obj_add_event_cb(beep_btn, handle_buzzer, LV_EVENT_ALL, nullptr);
        lv_obj_align(beep_btn, LV_ALIGN_TOP_MID, 0, 40);
        lv_obj_add_flag(beep_btn, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_set_height(beep_btn, LV_SIZE_CONTENT);

        label = lv_label_create(beep_btn);
        lv_label_set_text(label, "Beep");
        lv_obj_center(label);

        lv_obj_t *clr_rgb_btn = lv_btn_create(lv_scr_act());
        lv_obj_add_event_cb(clr_rgb_btn, handle_clr_rgb, LV_EVENT_ALL, nullptr);
        lv_obj_align(clr_rgb_btn, LV_ALIGN_TOP_MID, 0, 80);
        lv_obj_add_flag(clr_rgb_btn, LV_OBJ_FLAG_CHECKABLE);

        label = lv_label_create(clr_rgb_btn);
        lv_label_set_text(label, "Turn off RGB");
        lv_obj_center(label);

        // RGB

        // create a slider in the center of the display
        lv_obj_t *lv_colorwheel = lv_colorwheel_create(lv_scr_act(), true);
        lv_obj_set_size(lv_colorwheel, 200, 200);
        lv_obj_align(lv_colorwheel, LV_ALIGN_TOP_MID, 100, 0);

        lv_obj_center(lv_colorwheel);
        lv_obj_add_event_cb(lv_colorwheel, slider_event_cb, LV_EVENT_VALUE_CHANGED, nullptr);

        rgbw_init();

        buttons_init(&gpio_buttons_callback);

        led1 = lv_led_create(lv_scr_act());
        lv_obj_align(led1, LV_ALIGN_TOP_MID, -30, 400);
        lv_led_set_color(led1, lv_palette_main(LV_PALETTE_RED));

        lv_led_off(led1);

        led2 = lv_led_create(lv_scr_act());
        lv_obj_align(led2, LV_ALIGN_TOP_MID, 30, 400);
        lv_led_set_color(led2, lv_palette_main(LV_PALETTE_GREEN));

        lv_led_off(led2);

        user_leds_init();

        jy_label = lv_label_create(lv_scr_act());
        lv_label_set_text(jy_label, "X = 0 Y = 0");
        lv_obj_set_style_text_align(jy_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_width(jy_label, 50);
        lv_obj_align(jy_label, LV_ALIGN_CENTER, 0, 0);

        lv_obj_t *btn_label = lv_label_create(lv_scr_act());
        lv_label_set_text(btn_label, "Press Button to Toggle LED!");
        lv_obj_set_style_text_align(btn_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_align(btn_label, LV_ALIGN_BOTTOM_MID, 0, -20);
    }
}

void lv_example_btn_1()
{
    lv_obj_t *label;

    lv_obj_t *btn1 = lv_btn_create(lv_scr_act());
    lv_obj_add_event_cb(btn1, handle_keypad, LV_EVENT_ALL, nullptr);
    lv_obj_align(btn1, LV_ALIGN_TOP_MID, 0, 40);

    label = lv_label_create(btn1);
    lv_label_set_text(label, "KBD Demo");
    lv_obj_center(label);

    lv_obj_t *btn2 = lv_btn_create(lv_scr_act());
    lv_obj_add_event_cb(btn2, handle_hw, LV_EVENT_ALL, nullptr);
    lv_obj_align(btn2, LV_ALIGN_TOP_MID, 0, 80);

    label = lv_label_create(btn2);
    lv_label_set_text(label, "HW Demo");
    lv_obj_center(label);
}

static void unused_cb(uint __unused gpio, uint32_t __unused event_mask) {}

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


    static lv_coord_t col_dsc[] = {200, 240, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[] = {400, LV_GRID_TEMPLATE_LAST};
    lv_obj_set_style_grid_column_dsc_array(tab3, col_dsc, 0);
    lv_obj_set_style_grid_row_dsc_array(tab3, row_dsc, 0);
    // lv_obj_set_size(tab3, 480, 500);
    lv_obj_center(tab3);
    lv_obj_set_layout(tab3, LV_LAYOUT_GRID);

    label = lv_label_create(tab3);
    char product_id[512] = {0};

    const uint8_t rom_version = {rp2040_rom_version()};
    snprintf(product_id, sizeof(product_id),
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
    snprintf(product_id, sizeof(product_id),
             "Touch\n"
             " Product ID:  \t 0x%08" PRIx32 "\n"
             " Resolution:  \t %" PRIu16 " x %" PRIu16 "\n"
             " I2C Address: \t0x%" PRIx8 "\n"
             " Baud Rate:   \t %" PRIu32 "\n"
             " FW Version:  \t0x%08" PRIx16 "\n"
             " Vendor ID:  \t0x%02" PRIx8 "\n"
             "Display\n"
             " Resolution: \n"
             " SPI Address: \n"
             " Baud Rate: \n"
             " FW Version\n",
             globals.touch.product_id, globals.touch.width_px, globals.touch.height_px, globals.touch.i2c_address,
             globals.touch.baud_rate, globals.touch.firmware_version, globals.touch.vendor_id);
    lv_label_set_text(label, product_id);
    lv_obj_set_grid_cell(label, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
}

void example()
{
    img1 = lv_img_create(lv_scr_act());
    lv_img_set_src(img1, &ai);
    lv_obj_align(img1, LV_ALIGN_DEFAULT, 0, 0);
    lv_example_btn_1();
}

static void init()
{
    setup_default_uart();
    printf("\n**** RF Power Meter (Version " PICO_PROGRAM_VERSION_STRING " Built " __DATE__ ") ****\nmain_core0: init ...\n");

    stdio_init_all();

    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();
    rgbw_init();

    joystick_init(unused_cb);
    printf("main_core0: init done\n");
}

[[noreturn]]
void main_core0()
{
    init();

    repeating_timer_t timer;
    add_repeating_timer_ms(1, ms_tick_timer_cb, nullptr, &timer);

    lv_obj_clean(lv_scr_act());
    busy_wait_ms(100);

    default_tab_view();
    // example();

    while(true)
    {
        if(0 == systemTicksMs % 5) lv_task_handler();
    }
}
