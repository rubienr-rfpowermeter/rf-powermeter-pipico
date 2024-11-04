#pragma once
#include "Average.h"
#include <pico/mutex.h>

#include "TransactionData.h"

template <typename T> struct TransactionBufferT
{
  static constexpr uint8_t buffer_size = { 3 };

  TransactionBufferT();
  T    read();
  void write(const T &in);

private:

  volatile uint8_t write_index{ 1 };   /// cannot take over read index, stalls if no space to write
  volatile uint8_t read_index{ 0 };    /// cannot ake over write index, stalls if nothing to read

  T       buffer[buffer_size] = { 0 };
  mutex_t mutex;
};

template <typename T> TransactionBufferT<T>::TransactionBufferT() { mutex_init(&mutex); }

template <typename T> T TransactionBufferT<T>::read()
{
  mutex_enter_blocking(&mutex);
  const uint8_t next_index = { (uint8_t)((read_index + 1) % buffer_size) };
  read_index               = (next_index != write_index) ? next_index : read_index;
  mutex_exit(&mutex);

  return buffer[read_index];
}

template <typename T> void TransactionBufferT<T>::write(const T &in)
{
  mutex_enter_blocking(&mutex);
  const uint8_t next_index = { (uint8_t)((write_index + 1) % buffer_size) };
  write_index              = (next_index != read_index) ? next_index : write_index;
  mutex_exit(&mutex);

  buffer[write_index] = in;
};
