#include "lib/sample_data/TransactionBuffer.h"
#include "modules/core0_ui/core0_main.h"
#include "modules/core1_sampler/core1_main.h"
#include <pico/multicore.h>

TransactionBuffer transactionBuffer{};

[[noreturn]]
int main()
{
  core0_init(transactionBuffer);
  core1_init(transactionBuffer);

  multicore_launch_core1(core1_main);   // produce samples
  core0_main();                         // consume samples
}
