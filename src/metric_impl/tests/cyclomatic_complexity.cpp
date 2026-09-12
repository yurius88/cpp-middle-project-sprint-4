#include "metric_impl/cyclomatic_complexity.hpp"

#include <gtest/gtest.h>

#include "file.hpp"
#include "function.hpp"

namespace analyzer::metric::metric_impl {

using namespace analyzer::file;
using namespace analyzer::function;

class CyclomaticComplexityTest : public ::testing::Test {
protected:
    CyclomaticComplexityMetric metric;
    FunctionExtractor extractor;
};

TEST_F(CyclomaticComplexityTest, SimpleFunctionNoConditions) {
    File file("simple.py");
    auto functions = extractor.Get(file);
    ASSERT_FALSE(functions.empty());

    auto result = metric.Calculate(functions[0]);
    EXPECT_EQ(std::get<int>(result.value), 2);
}

TEST_F(CyclomaticComplexityTest, OneIfStatement) {
    File file("if.py");
    auto functions = extractor.Get(file);
    ASSERT_FALSE(functions.empty());

    auto result = metric.Calculate(functions[0]);
    EXPECT_EQ(std::get<int>(result.value), 2);
}

TEST_F(CyclomaticComplexityTest, NestedIfWithElif) {
    File file("nested_if.py");
    auto functions = extractor.Get(file);
    ASSERT_FALSE(functions.empty());

    auto result = metric.Calculate(functions[0]);
    EXPECT_EQ(std::get<int>(result.value), 5);
}

TEST_F(CyclomaticComplexityTest, LoopsWithConditions) {
    File file("loops.py");
    auto functions = extractor.Get(file);
    ASSERT_FALSE(functions.empty());

    auto result = metric.Calculate(functions[0]);
    EXPECT_EQ(std::get<int>(result.value), 4);
}

TEST_F(CyclomaticComplexityTest, TernaryOperator) {
    File file("ternary.py");
    auto functions = extractor.Get(file);
    ASSERT_FALSE(functions.empty());

    auto result = metric.Calculate(functions[0]);
    EXPECT_EQ(std::get<int>(result.value), 3);
}

TEST_F(CyclomaticComplexityTest, TryExceptFinally) {
    File file("exceptions.py");
    auto functions = extractor.Get(file);
    ASSERT_FALSE(functions.empty());

    auto result = metric.Calculate(functions[0]);
    EXPECT_EQ(std::get<int>(result.value), 4);
}

TEST_F(CyclomaticComplexityTest, MatchCase) {
    File file("match_case.py");
    auto functions = extractor.Get(file);
    ASSERT_FALSE(functions.empty());

    auto result = metric.Calculate(functions[0]);
    EXPECT_EQ(std::get<int>(result.value), 4);
}

TEST_F(CyclomaticComplexityTest, MetricName) {
    File file("simple.py");
    auto functions = extractor.Get(file);
    ASSERT_FALSE(functions.empty());

    auto result = metric.Calculate(functions[0]);
    EXPECT_EQ(result.metric_name, "Cyclomatic Complexity");
}

}  // namespace analyzer::metric::metric_impl
