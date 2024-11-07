#pragma once

#include "ad7887_types.h"
#include "lib/ad7887/registers.h"

void ad7887_init(Ad7887Sample &data);
void ad7887_start_transaction();
