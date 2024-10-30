#pragma once

#include "lib/sample_data/TransactionBuffer.h"

void              core0_init(TransactionBuffer &in_buffer);
[[noreturn]] void core0_main();
