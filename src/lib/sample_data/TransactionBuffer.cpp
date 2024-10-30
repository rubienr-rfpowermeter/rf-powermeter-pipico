#include "TransactionBuffer.h"

#include <cinttypes>
#include <cstdio>

TransactionBuffer::TransactionBuffer() { mutex_init(&mutex); }

ResultUint16 TransactionBuffer::read()
{
  mutex_enter_blocking(&mutex);
  const uint8_t next_index = { (uint8_t)((read_index + 1) % buffer_size) };
  read_index               = (next_index != write_index) ? next_index : read_index;
  mutex_exit(&mutex);

  return buffer[read_index];
}

void TransactionBuffer::write(ResultUint16 &in)
{
  mutex_enter_blocking(&mutex);
  const uint8_t next_index = { (uint8_t)((write_index + 1) % buffer_size) };
  write_index              = (next_index != read_index) ? next_index : write_index;
  mutex_exit(&mutex);

  buffer[write_index] = in;
};
