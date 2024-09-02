#include "modules/main_sampler/main.h"
#include "modules/main_ui/main.h"
#include <pico/multicore.h>

[[noreturn]]
int main()
{
    multicore_launch_core1(main_core1);
    main_core0();
}
