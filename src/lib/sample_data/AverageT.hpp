#pragma once

#include <cinttypes>
#include <cmath>
#include <numeric>

template <typename entry_t> struct ResultT
{
  entry_t value;
  entry_t avg;
  entry_t min;
  entry_t max;
};

template <uint8_t max_entries_count, typename item_type, typename cumulative_sum_type> struct AverageT
{
  static_assert(0 < max_entries_count);

  item_type put(item_type next_value)
  {
    const item_type out_value{ values[write_index] };
    values[write_index] = next_value;

    last_value_index = write_index;
    write_index      = (write_index + 1) % entries_count;

    cumulative_sum -= out_value;
    cumulative_sum += next_value;
    computeMinMax();

    return out_value;
  }

  ResultT<item_type> get() const
  {
    return ResultT<item_type>{
      .value = values[last_value_index], .avg = static_cast<item_type>(cumulative_sum / entries_count), .min = min, .max = max
    };
  }

  void clear(item_type default_value = 0)
  {
    cumulative_sum = 0;

    for (size_t i = 0; i < entries_count; i++)
      values[i] = default_value;
  }

  void updateCapacity(int8_t delta)
  {
    if (0 == delta) return;

    else if (delta < 0)   // decrease
    {
      if (abs(delta) >= entries_count) { entries_count = 1; }
      else entries_count += delta;
    }

    else if (delta > 0)   // increase
    {
      if (max_entries_count - delta <= entries_count) entries_count = max_entries_count;
      else entries_count += delta;
    }

    clear();
  }

  void setCapacity(uint8_t num_values)
  {
    clear();
    if (0 == num_values) entries_count = 1;
    else if (max_entries_count < num_values) entries_count = max_entries_count;
    else entries_count = num_values;
  }

  [[nodiscard]] uint8_t getCapacity() const { return entries_count; }

  [[nodiscard]] uint8_t getMaxCapacity() const { return max_entries_count; }

protected:

  void computeMinMax()
  {
    min = std::numeric_limits<item_type>::max();
    max = std::numeric_limits<item_type>::min();

    for (size_t i = 0; i < entries_count; ++i)
    {
      const item_type value = { values[i] };

      max = value > max ? value : max;
      min = value < min ? value : min;
    }
  }

  cumulative_sum_type cumulative_sum{ 0 };
  item_type           values[max_entries_count]{ 0 };
  uint8_t             write_index{ 0 };        /// points to the item the next store will operate at (oldest item: first out)
  uint8_t             last_value_index{ 0 };   /// point to the last stored item (newest item: last in)
  uint8_t             entries_count{ max_entries_count };
  item_type           min{ std::numeric_limits<item_type>::max() };
  item_type           max{ std::numeric_limits<item_type>::min() };
};
