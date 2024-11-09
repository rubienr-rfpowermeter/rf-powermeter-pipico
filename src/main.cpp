#include "lib/sample_data/TransactionBuffer.h"
#include "modules/core0_ui/core0_main.h"
#include "modules/core1_sampler/core1_main.h"
#include <pico/multicore.h>

TransactionBuffer core1_to_core0_transaction_buffer {};

[[noreturn]] int main()
{
  core0_init(core1_to_core0_transaction_buffer);
  core1_init(core1_to_core0_transaction_buffer);

  multicore_launch_core1(core1_main);   // produce samples
  core0_main();                         // consume samples
}
