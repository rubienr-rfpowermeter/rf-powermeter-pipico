#include "buttons.h"

#include <pico/binary_info.h>


void buttons_init(gpio_irq_callback_t callback)
{
    bi_decl_if_func_used(bi_program_feature("User Buttons"))                                          //
    bi_decl_if_func_used(bi_2pins_with_names(buttons_gpio_1, "Button 1", buttons_gpio_2, "Button 2")) //

    gpio_set_irq_enabled_with_callback(buttons_gpio_1, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, callback);
    gpio_set_irq_enabled_with_callback(buttons_gpio_2, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, callback);
}
