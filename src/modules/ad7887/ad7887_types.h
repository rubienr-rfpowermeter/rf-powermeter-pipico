#pragma once

#include "lib/ad7887/registers.h"

struct Ad7887Sample
{
  ad7887::ReceptionData16b data;
  volatile bool            is_data_ready{ false };
};
