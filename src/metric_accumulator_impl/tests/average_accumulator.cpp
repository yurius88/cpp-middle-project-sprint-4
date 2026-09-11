#include "metric_accumulator_impl/average_accumulator.hpp"

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

metric::MetricResult MakeResult(int value) { return metric::MetricResult{.metric_name = kMetricName, .value = value}; }

void AccumulateAll(AverageAccumulator &acc, const std::vector<int> &values) {
    rs::for_each(values | rv::transform(MakeResult), [&acc](const metric::MetricResult &r) { acc.Accumulate(r); });
}

}  // namespace

class AverageAccumulatorTest : public ::testing::Test {
protected:
    AverageAccumulator acc;
};

TEST_F(AverageAccumulatorTest, SingleValue) {
    AccumulateAll(acc, {5});
    acc.Finalize();
    EXPECT_DOUBLE_EQ(acc.Get(), 5.0);
}

TEST_F(AverageAccumulatorTest, MultipleValuesIntegerAverage) {
    AccumulateAll(acc, {2, 4, 6});
    acc.Finalize();
    EXPECT_DOUBLE_EQ(acc.Get(), 4.0);
}

TEST_F(AverageAccumulatorTest, MultipleValuesFractionalAverage) {
    AccumulateAll(acc, {1, 2});
    acc.Finalize();
    EXPECT_DOUBLE_EQ(acc.Get(), 1.5);
}

TEST_F(AverageAccumulatorTest, LargeRange) {
    // iota 1..100 -> average = 50.5
    auto values = rv::iota(1, 101) | rs::to<std::vector<int>>();
    AccumulateAll(acc, values);
    acc.Finalize();
    EXPECT_DOUBLE_EQ(acc.Get(), 50.5);
}

TEST_F(AverageAccumulatorTest, ZeroValues) {
    AccumulateAll(acc, {0, 0, 0});
    acc.Finalize();
    EXPECT_DOUBLE_EQ(acc.Get(), 0.0);
}

TEST_F(AverageAccumulatorTest, GetBeforeFinalizeThrows) {
    AccumulateAll(acc, {1, 2, 3});
    EXPECT_THROW(acc.Get(), std::runtime_error);
}

TEST_F(AverageAccumulatorTest, ResetClearsState) {
    AccumulateAll(acc, {10, 20});
    acc.Finalize();
    EXPECT_DOUBLE_EQ(acc.Get(), 15.0);

    acc.Reset();
    // After reset Get() must throw again because it is no longer finalized.
    EXPECT_THROW(acc.Get(), std::runtime_error);

    AccumulateAll(acc, {4, 8});
    acc.Finalize();
    EXPECT_DOUBLE_EQ(acc.Get(), 6.0);
}

}  // namespace analyzer::metric_accumulator::metric_accumulator_impl::test
