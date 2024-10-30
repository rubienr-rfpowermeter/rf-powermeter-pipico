#pragma once
#include "Average.h"
#include <pico/mutex.h>

struct TransactionBuffer
{
  constexpr static uint8_t buffer_size = { 3 };

  TransactionBuffer();
  ResultUint16 read();
  void         write(ResultUint16 &in);

private:

  volatile uint8_t write_index{ 1 };   /// cannot take over read index, stalls if no space to write
  volatile uint8_t read_index{ 0 };    /// cannot ake over write index, stalls if nothing to read
  ResultUint16     buffer[buffer_size] = { 0 };
  mutex_t          mutex;
};
