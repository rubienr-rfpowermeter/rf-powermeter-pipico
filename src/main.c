#include "demos/keypad_encoder/lv_demo_keypad_encoder.h"
#include "hardware/adc.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "lvgl.h"
#include "pico/sem.h"
#include "pico/stdlib.h"
#include "modules/ws2812/rgbw.h"
#include <pico/binary_info.h>
#include <stdio.h>

extern lv_img_dsc_t ai;
lv_obj_t *img1 = NULL;
lv_obj_t *led1 = NULL;
lv_obj_t *led2 = NULL;
lv_obj_t *jy_label = NULL;

const uint8_t gpio_user_led_1 = {16};
const uint8_t gpio_user_led_2 = {17};
const uint8_t gpio_buzzer = {13};
const uint8_t gpio_user_button_1 = {15};
const uint8_t gpio_user_button_2 = {14};

volatile uint32_t systemTicksMs = {0};
bool ms_tick_timer_cb(__unused struct repeating_timer *t)
{
    systemTicksMs++;
    lv_tick_inc(1);
    return true;
}

static void keypad_handler(lv_event_t *e)
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

static void beep_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_VALUE_CHANGED) { gpio_xor_mask(1 << gpio_buzzer); }
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

static void clr_rgb_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) { rgbw_put_pixel(rgbw_to_u32(0, 0, 0)); }
}

void gpio_callback(uint gpio, uint32_t __unused events)
{
    switch(gpio)
    {
    case gpio_user_button_1:
        lv_led_toggle(led1);
        gpio_xor_mask(1ul << 16);
        break;
    case gpio_user_button_2:
        lv_led_toggle(led2);
        gpio_xor_mask(1ul << 17);
        break;
    default:
        break;
    }
}

static void user_leds_init(void)
{
    bi_decl_if_func_used(bi_program_feature("USER LEDs (2x)"))
    bi_decl_if_func_used(bi_1pin_with_name(gpio_user_led_1, "LED1 (blue)"))
    bi_decl_if_func_used(bi_1pin_with_name(gpio_user_led_2, "LED2 (blue)"))

    gpio_init(gpio_user_led_1);
    gpio_init(gpio_user_led_2);
    gpio_set_dir(gpio_user_led_1, GPIO_OUT);
    gpio_set_dir(gpio_user_led_2, GPIO_OUT);

    gpio_put(gpio_user_led_1, 0);
    gpio_put(gpio_user_led_2, 0);
}

static void buzzer_init(void)
{
    bi_decl_if_func_used(bi_program_feature("Buzzer")) bi_decl_if_func_used(bi_1pin_with_name(gpio_buzzer, "Buzzer")) gpio_init(gpio_buzzer);
    gpio_set_dir(gpio_buzzer, GPIO_OUT);
}

static void user_buttons_init()
{
    bi_decl_if_func_used(bi_program_feature("User Buttons"))
    bi_decl_if_func_used(bi_2pins_with_names(gpio_user_button_1, "Button 1", gpio_user_button_2, "Button 2"))
    gpio_set_irq_enabled_with_callback(gpio_user_button_1, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(gpio_user_button_2, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
}
static void hw_handler(lv_event_t *e)
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
        lv_obj_add_event_cb(beep_btn, beep_handler, LV_EVENT_ALL, NULL);
        lv_obj_align(beep_btn, LV_ALIGN_TOP_MID, 0, 40);
        lv_obj_add_flag(beep_btn, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_set_height(beep_btn, LV_SIZE_CONTENT);

        label = lv_label_create(beep_btn);
        lv_label_set_text(label, "Beep");
        lv_obj_center(label);

        lv_obj_t *clr_rgb_btn = lv_btn_create(lv_scr_act());
        lv_obj_add_event_cb(clr_rgb_btn, clr_rgb_handler, LV_EVENT_ALL, NULL);
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
        lv_obj_add_event_cb(lv_colorwheel, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

        rgbw_init();
        user_buttons_init();

        gpio_set_irq_enabled_with_callback(22, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

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

void lv_example_btn_1(void)
{
    lv_obj_t *label;

    lv_obj_t *btn1 = lv_btn_create(lv_scr_act());
    lv_obj_add_event_cb(btn1, keypad_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(btn1, LV_ALIGN_TOP_MID, 0, 40);

    label = lv_label_create(btn1);
    lv_label_set_text(label, "KBD Demo");
    lv_obj_center(label);

    lv_obj_t *btn2 = lv_btn_create(lv_scr_act());
    lv_obj_add_event_cb(btn2, hw_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(btn2, LV_ALIGN_TOP_MID, 0, 80);

    label = lv_label_create(btn2);
    lv_label_set_text(label, "HW Demo");
    lv_obj_center(label);
}

void task0(void)
{

    lv_obj_clean(lv_scr_act());

    busy_wait_ms(100);

    img1 = lv_img_create(lv_scr_act());
    lv_img_set_src(img1, &ai);
    lv_obj_align(img1, LV_ALIGN_DEFAULT, 0, 0);
    lv_example_btn_1();
}

[[noreturn]]
int main()
{
    bi_decl_if_func_used(bi_program_feature("Joystick 2-axis (ADC0, ADC1)"))
    bi_decl_if_func_used(bi_1pin_with_name(26, "X-axis (ADC0)"))
    bi_decl_if_func_used(bi_1pin_with_name(27, "Y-axis (ADC1)"))

    stdio_init_all();
    setup_default_uart();

    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();
    rgbw_init();

    struct repeating_timer timer;
    add_repeating_timer_ms(1, ms_tick_timer_cb, NULL, &timer);

    task0();
    while(1)
    {
        if(systemTicksMs % 5 == 0) lv_task_handler();
    }
}
