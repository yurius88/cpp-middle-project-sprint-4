#include "metric_impl/code_lines_count.hpp"

#include <gtest/gtest.h>

#include "file.hpp"
#include "function.hpp"

namespace analyzer::metric::metric_impl {

using namespace analyzer::file;
using namespace analyzer::function;

class CodeLinesCountTest : public ::testing::Test {
protected:
    CodeLinesCountMetric metric;
    FunctionExtractor extractor;
};

TEST_F(CodeLinesCountTest, BasicCheck) { EXPECT_EQ(1 + 1, 2); }

TEST_F(CodeLinesCountTest, SimpleFunction) {
    File file("simple.py");
    auto functions = extractor.Get(file);
    ASSERT_FALSE(functions.empty());

    auto result = metric.Calculate(functions[0]);
    EXPECT_EQ(std::get<int>(result.value), 5);
}

TEST_F(CodeLinesCountTest, FunctionWithComments) {
    File file("comments.py");
    auto functions = extractor.Get(file);
    ASSERT_FALSE(functions.empty());

    auto result = metric.Calculate(functions[0]);
    EXPECT_EQ(std::get<int>(result.value), 3);
}

TEST_F(CodeLinesCountTest, ManyLines) {
    File file("many_lines.py");
    auto functions = extractor.Get(file);
    ASSERT_FALSE(functions.empty());

    auto result = metric.Calculate(functions[0]);
    EXPECT_GT(std::get<int>(result.value), 5);
}

TEST_F(CodeLinesCountTest, IfStatement) {
    File file("if.py");
    auto functions = extractor.Get(file);
    ASSERT_FALSE(functions.empty());

    auto result = metric.Calculate(functions[0]);
    EXPECT_EQ(std::get<int>(result.value), 3);
}

TEST_F(CodeLinesCountTest, MetricName) {
    File file("simple.py");
    auto functions = extractor.Get(file);
    ASSERT_FALSE(functions.empty());

    auto result = metric.Calculate(functions[0]);
    EXPECT_EQ(result.metric_name, "Code lines count");
}

}  // namespace analyzer::metric::metric_impl
