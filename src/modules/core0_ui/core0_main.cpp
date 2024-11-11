#include "core0_main.h"

#include "lib/sample_data/TransactionBuffer.h"
#include "modules/lvgl/lv_display.h"
#include "modules/lvgl/lv_input.h"
#include "modules/periphery/display/display.h"
#include "modules/periphery/input/buttons.h"
#include "modules/periphery/input/input.h"
#include "modules/periphery/input/joystick.h"
#include "modules/periphery/led/user_led.h"
#include "ui.h"
#include <cstdio>
#include <hardware/clocks.h>
#include <lvgl.h>
#include <pico/multicore.h>
#include <pico/stdlib.h>

struct SamplingResources
{
  TransactionBuffer *in_buffer{ nullptr };
  TransactionData    sample;
};

static SamplingResources sampling{};
static volatile uint32_t system_ticks_ms{ 0 };

static bool ms_tick_timer_cb(__unused struct repeating_timer *t)
{
  system_ticks_ms += 1;
  lv_tick_inc(1);
  return true;
}

static void uart_post_init()
{
  constexpr uint8_t gpio_uart_tx{ PICO_DEFAULT_UART_TX_PIN };
  constexpr uint8_t gpio_uart_rx{ PICO_DEFAULT_UART_RX_PIN };
  gpio_disable_pulls(gpio_uart_tx);
  gpio_disable_pulls(gpio_uart_rx);
  gpio_set_drive_strength(gpio_uart_tx, GPIO_DRIVE_STRENGTH_2MA);
  gpio_set_slew_rate(gpio_uart_tx, GPIO_SLEW_RATE_SLOW);
}

void on_gpio_edge(uint gpio, uint32_t event_mask)
{
  if (buttons_on_state_changed_cb(gpio, event_mask)) return;
  if (joystick_on_edge_cb(gpio, event_mask)) return;
}

static void init(TransactionBuffer &in_buffer)
{
  printf("\nC0I **** RF Power Meter (Version " PICO_PROGRAM_VERSION_STRING " Built " __DATE__ ") ****\n");
  printf("C0I init ...\n");
  printf("C0I sys_clk_hz=%" PRIu32 "\n", clock_get_hz(clk_sys));

  gpio_set_irq_callback(&on_gpio_edge);
  user_led_init();
  buttons_init();
  joystick_init();
  input_init();

  display_init();
  lv_display_init();
  static TrackedInputs input_keys;
  lv_input_init(input_keys);

  sampling.in_buffer = &in_buffer;
  ui_init(sampling.sample);

  printf("C0I init done\n");
}

void core0_init(TransactionBuffer &in_buffer)
{
  stdio_init_all();
  uart_post_init();
  init(in_buffer);
}

[[noreturn]]
void core0_main()
{
  assert(0 == get_core_num());

  // init();

  { // todo rr - find out why this fishy workaround was necessary (repair hotfix)
    lv_tick_inc(1);
    lv_task_handler();
  }

  constexpr uint8_t sync_signal{ 42 };
  printf("C0I main sending sync. signal %" PRIu8 " to other core ...\n", sync_signal);
  multicore_fifo_push_blocking(sync_signal);
  printf("C0I main sync signal %" PRIu8 " sent other core\n", sync_signal);

  repeating_timer_t timer;
  add_repeating_timer_ms(1, ms_tick_timer_cb, nullptr, &timer);

  while (true)
  {
    if (0 == system_ticks_ms % 5)
    {
      sampling.sample = sampling.in_buffer->read();
      ui_update();
      lv_task_handler();
    }
  }
}
