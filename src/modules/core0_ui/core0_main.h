#pragma once

#include "lib/sample_data/TransactionBuffer.h"

void              core0_init(TransactionBuffer &samples);
[[noreturn]] void core0_main();
