#pragma once

struct TransactionBuffer;

void              core0_init(TransactionBuffer &samples);
[[noreturn]] void core0_main();
