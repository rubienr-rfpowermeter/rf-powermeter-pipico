#include "lib/sample_data/Average.h"

#include <gtest/gtest.h>
#include <vector>

using UnderlyingAvgType = double;
using AverageF8         = AverageT<8, UnderlyingAvgType, UnderlyingAvgType>;
using SampleF           = ResultT<UnderlyingAvgType>;

struct AverageTest : public testing::Test, public AverageF8
{

  void updateTestData(const std::vector<UnderlyingAvgType> &input_data)
  {
    for (const auto &v : input_data)
      put(v);
  }
};

TEST_F(AverageTest, cumulative_sum_a)
{
  updateTestData({ 1, 2, 3, 4, 5, 6, 7, 8 });
  EXPECT_FLOAT_EQ(1 + 2 + 3 + 4 + 5 + 6 + 7 + 8, cumulative_sum);
}

TEST_F(AverageTest, cumulative_sum_b)
{
  updateTestData({ 1, 2, 3, 4, 5, 6, 7, 8,   //
                   8, 8, 8, 8, 8, 8 });
  EXPECT_FLOAT_EQ(7 + 7 * 8, cumulative_sum);
}

TEST_F(AverageTest, cumulative_sum_c)
{
  updateTestData({ 1, 2, 3, 4, 5, 6, 7, 8,   //
                   8, 8, 8, 8, 8, 8, 8, 8 });
  EXPECT_FLOAT_EQ(8 * 8, cumulative_sum);
}

TEST_F(AverageTest, average_a)
{
  updateTestData({ 1, 2, 3, 4, 5, 6, 7 });
  EXPECT_EQ(8, entries_count);

  SampleF s {get()};
  EXPECT_FLOAT_EQ((1 + 2 + 3 + 4 + 5 + 6 + 7 + 0) / 8.0f, s.avg);
}

TEST_F(AverageTest, average_b)
{
  updateCapacity(-2);

  updateTestData({ 1, 2, 3, 4, 5, 6 });
  EXPECT_EQ(6, entries_count);

  SampleF s {get()};
  EXPECT_FLOAT_EQ((1 + 2 + 3 + 4 + 5 + 6) / 6.0f, s.avg);
}

TEST_F(AverageTest, set_capacity_a)
{
  setCapacity(2);

  updateTestData({ 1, 2, 3, 4, 5, 6 });
  EXPECT_EQ(2, entries_count);

  SampleF s {get()};
  EXPECT_FLOAT_EQ((5 + 6) / 2.0f, s.avg);
}

TEST_F(AverageTest, upate_capacity_a)
{
  setCapacity(2);
  updateCapacity(-1);

  updateTestData({ 1, 2, 3, 4, 5, 6 });
  EXPECT_EQ(1, entries_count);

  SampleF s {get()};
  EXPECT_FLOAT_EQ(6 / 1.0f, s.avg);
}

TEST_F(AverageTest, upate_capacity_b)
{
  setCapacity(1);
  updateCapacity(-1);

  updateTestData({ 1, 2, 3, 4, 5, 6 });
  EXPECT_EQ(1, entries_count);

  SampleF s {get()};
  EXPECT_FLOAT_EQ(6 / 1.0f, s.avg);
}

TEST_F(AverageTest, upate_capacity_c)
{
  setCapacity(9);
  updateCapacity(1);

  updateTestData({ 1, 2, 3, 4, 5, 6, 7, 8 });
  EXPECT_EQ(8, entries_count);

  SampleF s {get()};
  EXPECT_FLOAT_EQ((1 + 2 + 3 + 4 + 5 + 6 + 7 + 8) / 8.0f, s.avg);
}

TEST_F(AverageTest, random_values_a)
{
  std::vector<UnderlyingAvgType> random_values(8);
  generate(random_values.begin(), random_values.end(), rand);

  auto sum                        = accumulate(begin(random_values), end(random_values), static_cast<UnderlyingAvgType>(0));
  auto [min, max]                 = minmax_element(begin(random_values), end(random_values));
  const UnderlyingAvgType average = { sum / static_cast<UnderlyingAvgType>(8) };

  updateTestData(random_values);
  EXPECT_EQ(8, entries_count);

  SampleF s {get()};
  EXPECT_FLOAT_EQ(sum, cumulative_sum);
  EXPECT_FLOAT_EQ(average, s.avg);
  EXPECT_FLOAT_EQ(*min, s.min);
  EXPECT_FLOAT_EQ(*max, s.max);
}

TEST_F(AverageTest, random_values_b)
{
  std::vector<UnderlyingAvgType> random_values(1024);
  generate(begin(random_values), end(random_values), rand);

  updateTestData(random_values);

  std::vector<UnderlyingAvgType> random_values_tail(next(end(random_values), -8), end(random_values));

  auto sum        = accumulate(begin(random_values_tail), end(random_values_tail), static_cast<UnderlyingAvgType>(0));
  auto [min, max] = minmax_element(begin(random_values_tail), end(random_values_tail));

  EXPECT_EQ(8, abs(distance(begin(random_values_tail), end(random_values_tail))));

  const UnderlyingAvgType average = { sum / static_cast<UnderlyingAvgType>(8) };
  EXPECT_EQ(8, entries_count);

  SampleF s {get()};
  EXPECT_FLOAT_EQ(sum, cumulative_sum);
  EXPECT_FLOAT_EQ(average, s.avg);
  EXPECT_FLOAT_EQ(*min, s.min);
  EXPECT_FLOAT_EQ(*max, s.max);
}
