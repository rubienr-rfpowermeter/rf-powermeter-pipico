#pragma once

#include "lib/sample_data/TransactionBuffer.h"

void              core1_init(TransactionBuffer &out_buffer);
[[noreturn]] void core1_main();
