#include "rgbw.h"

#include "hardware/pio.h"
#include "ws2812.pio.h"
#include <pico/binary_info.h>

PIO rgbw_pio = {pio0};
int rgbw_sm = {0};
int rgbw_offset = {0};

void rgbw_put_pixel(uint32_t pixel_grb) { pio_sm_put_blocking(rgbw_pio, rgbw_sm, pixel_grb << 8u); }

uint32_t rgbw_to_u32(uint8_t r, uint8_t g, uint8_t b) { return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b); }

void rgbw_init(void)
{
    const uint8_t rgb_gpio = {12};

    bi_decl_if_func_used(bi_program_feature("RGB LED 1x (WS2012, PIO0)"));
    bi_decl_if_func_used(bi_1pin_with_func(rgb_gpio, GPIO_FUNC_PIO0));
    bi_decl_if_func_used(bi_1pin_with_name(rgb_gpio, "RGB LED"));

    rgbw_offset = pio_add_program(rgbw_pio, &ws2812_program);
    ws2812_program_init(rgbw_pio, rgbw_sm, rgbw_offset, rgb_gpio, 800000, true);
    rgbw_put_pixel(rgbw_to_u32(0, 0, 0));
}
