#include "joystick.h"

#include <pico/binary_info.h>


void joystick_init(gpio_irq_callback_t __unused callback)
{
    bi_decl_if_func_used(bi_program_feature("Joystick 2-axis (ADC0, ADC1)"))  //
    bi_decl_if_func_used(bi_1pin_with_name(joystick_axis_x, "X-axis (ADC0)")) //
    bi_decl_if_func_used(bi_1pin_with_name(joystick_axis_y, "Y-axis (ADC1)")) //

    // TODO
}
