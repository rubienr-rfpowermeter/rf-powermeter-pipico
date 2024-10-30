#include "main.h"

#include "lib/sample_data/TransactionBuffer.h"
#include <cinttypes>
#include <cstdio>
#include <pico/multicore.h>

static TransactionBuffer *samples         = { nullptr };
static volatile uint32_t  system_ticks_ms = { 0 };

AverageUint16 average;

static bool __unused ms_tick_timer_cb(__unused struct repeating_timer *t)
{
  system_ticks_ms += 1;
  average.put((uint16_t)(sin(system_ticks_ms) * 100u));

  if (0 == system_ticks_ms % 5u)
  {
    ResultUint16 r;
    average.get(r);
    samples->write(r);
  }

  return true;
}

static void init()
{

  const uint32_t received_value = { multicore_fifo_pop_blocking() };
  printf("c%" PRIu8 " init synchronized on signal %" PRIu32 "\n", get_core_num(), received_value);

  printf("init_core%" PRIu8 " ...\n", get_core_num());

  printf("init_core%" PRIu8 " done\n", get_core_num());
}

void core1_init(TransactionBuffer &buffer) { samples = &buffer; }

[[noreturn]]
void core1_main()
{
  init();

  repeating_timer_t timer;
  add_repeating_timer_ms(1, ms_tick_timer_cb, nullptr, &timer);
  while (true) { }
}
