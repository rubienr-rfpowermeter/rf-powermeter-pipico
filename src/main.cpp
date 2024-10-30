#include "modules/main_sampler/main.h"
#include "lib/sample_data/TransactionBuffer.h"
#include "modules/main_ui/main.h"
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
