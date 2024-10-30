#include "modules/main_sampler/main.h"
#include "lib/sample_data/TransactionBuffer.h"
#include "modules/main_ui/main.h"
#include <pico/multicore.h>

TransactionBuffer samples;

[[noreturn]]
int main()
{
  core0_init(samples);
  core1_init(samples);

  multicore_launch_core1(core1_main);
  core0_main();
}
