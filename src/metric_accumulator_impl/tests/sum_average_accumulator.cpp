#include "metric_accumulator_impl/sum_average_accumulator.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <ranges>
#include <stdexcept>
#include <vector>

namespace analyzer::metric_accumulator::metric_accumulator_impl::test {

namespace rv = std::ranges::views;
namespace rs = std::ranges;

namespace {

constexpr char kMetricName[] = "some metric";

metric::MetricResult MakeResult(int value) {
    return metric::MetricResult{.metric_name = kMetricName, .value = value};
}

void AccumulateAll(SumAverageAccumulator &acc, const std::vector<int> &values) {
    rs::for_each(values | rv::transform(MakeResult),
                 [&acc](const metric::MetricResult &r) { acc.Accumulate(r); });
}

}  // namespace

class SumAverageAccumulatorTest : public ::testing::Test {
protected:
    SumAverageAccumulator acc;
};

TEST_F(SumAverageAccumulatorTest, SingleValue) {
    AccumulateAll(acc, {7});
    acc.Finalize();
    auto result = acc.Get();
    EXPECT_EQ(result.sum, 7);
    EXPECT_DOUBLE_EQ(result.average, 7.0);
}

TEST_F(SumAverageAccumulatorTest, MultipleValuesIntegerAverage) {
    AccumulateAll(acc, {2, 4, 6});
    acc.Finalize();
    auto result = acc.Get();
    EXPECT_EQ(result.sum, 12);
    EXPECT_DOUBLE_EQ(result.average, 4.0);
}

TEST_F(SumAverageAccumulatorTest, MultipleValuesFractionalAverage) {
    AccumulateAll(acc, {1, 2});
    acc.Finalize();
    auto result = acc.Get();
    EXPECT_EQ(result.sum, 3);
    EXPECT_DOUBLE_EQ(result.average, 1.5);
}

TEST_F(SumAverageAccumulatorTest, LargeRange) {
    // iota 1..100 -> sum = 5050, average = 50.5
    auto values = rv::iota(1, 101) | rs::to<std::vector<int>>();
    AccumulateAll(acc, values);
    acc.Finalize();
    auto result = acc.Get();
    EXPECT_EQ(result.sum, 5050);
    EXPECT_DOUBLE_EQ(result.average, 50.5);
}

TEST_F(SumAverageAccumulatorTest, SpaceshipComparison) {
    AccumulateAll(acc, {2, 4, 6});
    acc.Finalize();
    SumAverageAccumulator::SumAverage expected{.sum = 12, .average = 4.0};
    EXPECT_EQ(acc.Get(), expected);
}

TEST_F(SumAverageAccumulatorTest, GetBeforeFinalizeThrows) {
    AccumulateAll(acc, {1, 2, 3});
    EXPECT_THROW(acc.Get(), std::runtime_error);
}

TEST_F(SumAverageAccumulatorTest, ResetClearsState) {
    AccumulateAll(acc, {10, 20});
    acc.Finalize();
    {
        auto result = acc.Get();
        EXPECT_EQ(result.sum, 30);
        EXPECT_DOUBLE_EQ(result.average, 15.0);
    }

    acc.Reset();
    EXPECT_THROW(acc.Get(), std::runtime_error);

    AccumulateAll(acc, {4, 8});
    acc.Finalize();
    {
        auto result = acc.Get();
        EXPECT_EQ(result.sum, 12);
        EXPECT_DOUBLE_EQ(result.average, 6.0);
    }
}

}  // namespace analyzer::metric_accumulator::metric_accumulator_impl::test
