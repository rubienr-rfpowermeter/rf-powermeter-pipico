#pragma once

#include "lib/ad8318/Converter3rdOrder.h"
#include "lib/sample_data/TransactionData.h"
#include <cinttypes>

void ui_init(TransactionData &samples,Converter3rdOrder &converter);
void ui_update();
