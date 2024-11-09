#pragma once

#include "ad7887_types.h"
#include "lib/ad7887/registers.h"

/// Starts up continuous fetching of data from AD7887 via DMA.
///
/// Read/write DMAs are started from PWM-DMA that is pacing every 1ms with PWM's wrap DREQ.
/// The PWM output can be observed on GPIO14.
/// @param data data output
void ad7887_init(Ad7887Sample &data);

/// Starts reading (starts PWM)
void ad7887_start();
