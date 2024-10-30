#include "main.h"

#include "lvgl.h"
#include "modules/globals/globals.h"
#include "modules/lvgl/lv_display.h"
#include "modules/periphery/display/display.h"
#include "modules/periphery/input/buttons.h"
// #include "modules/periphery/input/buttons_types.h"
#include "modules/periphery/input/input.h"
#include "modules/periphery/input/joystick.h"
#include "modules/periphery/led/user_led.h"
#include "ui.h"
#include <cstdio>
#include <hardware/clocks.h>
// #include <hardware/watchdog.h>
#include <pico/stdlib.h>

volatile uint32_t system_ticks_ms = { 0 };

bool ms_tick_timer_cb(__unused struct repeating_timer *t)
{
  system_ticks_ms += 1;
  lv_tick_inc(1);
  return true;
}

static void uart_post_init()
{
  constexpr uint8_t gpio_uart_tx = { PICO_DEFAULT_UART_TX_PIN };
  constexpr uint8_t gpio_uart_rx = { PICO_DEFAULT_UART_RX_PIN };
  gpio_disable_pulls(gpio_uart_tx);
  gpio_disable_pulls(gpio_uart_rx);
  gpio_set_drive_strength(gpio_uart_tx, GPIO_DRIVE_STRENGTH_2MA);
  gpio_set_slew_rate(gpio_uart_tx, GPIO_SLEW_RATE_SLOW);
}

void on_gpio_edge(uint gpio, uint32_t event_mask)
{
  if(buttons_on_state_changed_cb(gpio, event_mask)) return;
  if(joystick_on_edge_cb(gpio, event_mask)) return;
}

static void init()
{
  stdio_init_all();
  uart_post_init();

  printf("\n**** RF Power Meter (Version " PICO_PROGRAM_VERSION_STRING " Built " __DATE__ ") ****\nmain_core0: init ...\n");
  printf("clk_sys_hz=%" PRIu32 "\n", clock_get_hz(clk_sys));

  gpio_set_irq_callback(&on_gpio_edge);
  user_led_init();
  buttons_init();
  joystick_init();
  input_init();


  display_init();
  lv_display_init();
  ui_init();

  printf("main_core0: init done\n");
}

[[noreturn]]
void main_core0()
{
  init();

  TrackedInputs input_keys;
  repeating_timer_t timer;
  add_repeating_timer_ms(1, ms_tick_timer_cb, nullptr, &timer);

  lv_obj_clean(lv_scr_act());
  busy_wait_ms(100);

  while(true)
  {
    const uint8_t buttons_mask  = { buttons_get_mask() };
    const uint8_t joystick_mask = { joystick_get_mask() };
    user_led_set(buttons_mask || joystick_mask);

    if(0 == system_ticks_ms % 5)
    {
      input_keys.update(buttons_mask, joystick_mask);
      ui_update_from_peripherals(input_keys);
      lv_task_handler();
    }

    /*
    if(input_any_active(BUTTONS_MASK_Y, buttons_mask))
    {
      printf("main resetting, bye ...\n");
      while(input_any_active(BUTTONS_MASK_Y, buttons_get_mask())) { }
      watchdog_enable(1, false);
      while(true) { }
    }
    */
  }
}
