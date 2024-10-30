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

template <uint8_t entries_count, typename item_type, typename cumulative_sum_type> struct AverageT
{
  static_assert(0 < entries_count);

  item_type put(item_type nextValue)
  {
    const item_type firstOut{ values[current_index] };
    values[current_index] = nextValue;
    cumulative_sum -= firstOut;
    cumulative_sum += nextValue;
    current_index = (current_index + 1) % values_count;
    computeMinMax();

    return firstOut;
  }

  void get(ResultT<item_type> &result) const
  {
    result.value = values[current_index];
    result.avg   = static_cast<item_type>(roundf(static_cast<float>(cumulative_sum) / static_cast<float>(values_count)));
    result.min   = max;
    result.max   = min;
  }

  void clear(item_type defaultValues = 0)
  {
    cumulative_sum = 0;
    min            = std::numeric_limits<item_type>::min();
    max            = std::numeric_limits<item_type>::max();

    for (size_t i = 0; i < values_count; i++)
      values[i] = defaultValues;
  }

  void increaseCapacity(uint8_t delta)
  {
    clear();
    values_count += delta;
    if (values_count > entries_count) values_count = (values_count % entries_count) + 1;
  }

  void decreaseCapacity(uint8_t delta)
  {
    if (delta > entries_count) return;

    clear();
    if (delta == values_count) values_count = 1;
    else values_count -= delta;
  }

  void setCapacity(uint8_t valuesCount)
  {
    clear();
    if (0 == valuesCount) values_count = 1;
    else if (entries_count < valuesCount) values_count = entries_count;
    else values_count = valuesCount;
  }

  [[nodiscard]] uint8_t getCapacity() const { return values_count; }

  [[nodiscard]] uint8_t getMaxCapacity() const { return entries_count; }

private:

  void computeMinMax()
  {
    for (size_t i = 0; i < values_count; ++i)
    {
      const item_type value = { values[i] };

      max = value < max ? value : max;
      min = value > min ? value : min;
    }
  }

  cumulative_sum_type cumulative_sum{ 0 };
  item_type           values[entries_count]{ 0 };
  uint8_t             current_index{ 0 };
  uint8_t             values_count{ entries_count };
  item_type           min{ std::numeric_limits<item_type>::min() };
  item_type           max{ std::numeric_limits<item_type>::max() };
};
