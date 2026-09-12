#include "metric_accumulator_impl/categorical_accumulator.hpp"

#include <gtest/gtest.h>

#include <ranges>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace analyzer::metric_accumulator::metric_accumulator_impl::test {

namespace rv = std::ranges::views;
namespace rs = std::ranges;

namespace {

constexpr char kMetricName[] = "naming style";

metric::MetricResult MakeResult(const std::string &value) {
    return metric::MetricResult{.metric_name = kMetricName, .value = value};
}

void AccumulateAll(CategoricalAccumulator &acc, const std::vector<std::string> &values) {
    rs::for_each(values | rv::transform(MakeResult),
                 [&acc](const metric::MetricResult &r) { acc.Accumulate(r); });
}

}  // namespace

class CategoricalAccumulatorTest : public ::testing::Test {
protected:
    CategoricalAccumulator acc;
};

TEST_F(CategoricalAccumulatorTest, SingleCategory) {
    AccumulateAll(acc, {"snake_case"});
    acc.Finalize();
    const auto &freq = acc.Get();
    ASSERT_EQ(freq.size(), 1u);
    EXPECT_EQ(freq.at("snake_case"), 1);
}

TEST_F(CategoricalAccumulatorTest, SameCategoryCounted) {
    AccumulateAll(acc, {"snake_case", "snake_case", "snake_case"});
    acc.Finalize();
    const auto &freq = acc.Get();
    ASSERT_EQ(freq.size(), 1u);
    EXPECT_EQ(freq.at("snake_case"), 3);
}

TEST_F(CategoricalAccumulatorTest, MultipleCategories) {
    AccumulateAll(acc, {"snake_case", "camelCase", "snake_case", "PascalCase", "camelCase",
                        "snake_case"});
    acc.Finalize();
    const auto &freq = acc.Get();
    ASSERT_EQ(freq.size(), 3u);
    EXPECT_EQ(freq.at("snake_case"), 3);
    EXPECT_EQ(freq.at("camelCase"), 2);
    EXPECT_EQ(freq.at("PascalCase"), 1);
}

TEST_F(CategoricalAccumulatorTest, EmptyWhenNoAccumulation) {
    acc.Finalize();
    const auto &freq = acc.Get();
    EXPECT_TRUE(freq.empty());
}

TEST_F(CategoricalAccumulatorTest, GetBeforeFinalizeThrows) {
    AccumulateAll(acc, {"snake_case"});
    EXPECT_THROW(acc.Get(), std::runtime_error);
}

TEST_F(CategoricalAccumulatorTest, ResetClearsState) {
    AccumulateAll(acc, {"snake_case", "camelCase"});
    acc.Finalize();
    ASSERT_EQ(acc.Get().size(), 2u);

    acc.Reset();
    EXPECT_THROW(acc.Get(), std::runtime_error);

    AccumulateAll(acc, {"PascalCase", "PascalCase"});
    acc.Finalize();
    const auto &freq = acc.Get();
    ASSERT_EQ(freq.size(), 1u);
    EXPECT_EQ(freq.at("PascalCase"), 2);
}

}  // namespace analyzer::metric_accumulator::metric_accumulator_impl::test
