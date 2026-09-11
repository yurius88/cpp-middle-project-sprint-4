#include "metric_impl/parameters_count.hpp"

#include <gtest/gtest.h>

#include "file.hpp"
#include "function.hpp"

namespace analyzer::metric::metric_impl {

using namespace analyzer::file;
using namespace analyzer::function;

class ParametersCountTest : public ::testing::Test {
protected:
    CountParametersMetric metric;
    FunctionExtractor extractor;
};

TEST_F(ParametersCountTest, SimpleFunction) {
    File file("simple.py");
    auto functions = extractor.Get(file);
    ASSERT_FALSE(functions.empty());

    auto result = metric.Calculate(functions[0]);
    EXPECT_EQ(std::get<int>(result.value), 0);
}

TEST_F(ParametersCountTest, OneParameter) {
    File file("if.py");
    auto functions = extractor.Get(file);
    ASSERT_FALSE(functions.empty());

    auto result = metric.Calculate(functions[0]);
    EXPECT_EQ(std::get<int>(result.value), 1);
}

TEST_F(ParametersCountTest, MultipleParameters) {
    File file("comments.py");
    auto functions = extractor.Get(file);
    ASSERT_FALSE(functions.empty());

    auto result = metric.Calculate(functions[0]);
    EXPECT_EQ(std::get<int>(result.value), 3);
}

TEST_F(ParametersCountTest, ManyParameters) {
    File file("many_parameters.py");
    auto functions = extractor.Get(file);
    ASSERT_FALSE(functions.empty());

    auto result = metric.Calculate(functions[0]);
    // __test_multiparameters__(a, b, c=5, *args, **kwargs) - 5 параметров
    EXPECT_EQ(std::get<int>(result.value), 5);
}

TEST_F(ParametersCountTest, MetricName) {
    File file("simple.py");
    auto functions = extractor.Get(file);
    ASSERT_FALSE(functions.empty());

    auto result = metric.Calculate(functions[0]);
    EXPECT_EQ(result.metric_name, "Parameters count");
}

}  // namespace analyzer::metric::metric_impl
