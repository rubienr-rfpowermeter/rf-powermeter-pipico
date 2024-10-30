#pragma once

#include "lib/ad7887/registers.h"
#include "ad7887_types.h"

void ad7887_init(Ad7887Sample &data);
void ad7887_update();
