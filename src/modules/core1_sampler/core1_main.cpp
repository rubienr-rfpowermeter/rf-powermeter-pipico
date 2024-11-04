#include "core1_main.h"

#include "lib/sample_data/TransactionBuffer.h"
#include "modules/ad7887/ad7887.h"
#include <cinttypes>
#include <cstdio>
#include <pico/multicore.h>

static volatile uint32_t  system_ticks_ms{ 0 };
static TransactionBuffer *out_buffer{ nullptr };

struct Sampling
{
  Ad7887Sample  last_sample{};
  Average       average{};
  volatile bool request_new_data{ false };
};

static Sampling sampling{};

static bool ms_tick_timer_cb(__unused struct repeating_timer *t)
{
  system_ticks_ms += 1;
  if (0 == system_ticks_ms % 5u) sampling.request_new_data = true;
  return true;
}

static void init()
{

  const uint32_t received_value{ multicore_fifo_pop_blocking() };
  printf("c%" PRIu8 " init synchronized on signal %" PRIu32 "\n", get_core_num(), received_value);

  printf("init_core%" PRIu8 " ...\n", get_core_num());
  ad7887_init(sampling.last_sample);
  printf("init_core%" PRIu8 " done\n", get_core_num());
}

void core1_init(TransactionBuffer &out_buff) { out_buffer = &out_buff; }

[[noreturn]] void core1_main()
{
  init();

  alarm_pool_t *pool = alarm_pool_create_with_unused_hardware_alarm(8);

  repeating_timer_t timer;
  alarm_pool_add_repeating_timer_ms(pool, 1, ms_tick_timer_cb, nullptr, &timer);
  // add_repeating_timer_ms(1, ms_tick_timer_cb, nullptr, &timer);
  while (true)
  {

    if (sampling.request_new_data)
    {
      sampling.request_new_data          = false;
      sampling.last_sample.is_data_ready = false;
      ad7887_update();
    }

    if (sampling.last_sample.is_data_ready)
    {
      sampling.last_sample.is_data_ready = false;

      sampling.average.put(sampling.last_sample.data.asSampleRegister16b.raw12b);
      TransactionData td{ .timestamp_ms = system_ticks_ms, .raw_sample = { sampling.average.get() } };
      out_buffer->write(td);
    }
  }
}
