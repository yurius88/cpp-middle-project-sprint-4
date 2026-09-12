#include "metric_impl/cyclomatic_complexity.hpp"

#include <unistd.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <ranges>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

namespace analyzer::metric::metric_impl {
std::string CyclomaticComplexityMetric::Name() const { return kName; }
MetricResult::ValueType CyclomaticComplexityMetric::CalculateImpl(const function::Function &f) const {
    // Получаем строковое представление AST (абстрактного синтаксического дерева) функции.
    // Это S-выражение, сгенерированное утилитой tree-sitter, например:
    // "(function_definition name: (identifier) ... (if_statement ...) (for_statement ...))"
    auto &function_ast = f.ast;

    if (function_ast.empty()) {
        throw std::runtime_error("CyclomaticComplexityMetric: empty AST for function " + f.name);
    }

    // Список типов узлов AST, каждый из которых увеличивает цикломатическую сложность на 1.
    // Эти узлы соответствуют управляющим конструкциям языка Python:
    // - if / elif
    // - циклы (for, while)
    // - обработка исключений (try, finally)
    // - case в match-выражениях
    // - assert
    // - тернарный оператор (conditional_expression)
    constexpr std::array<std::string_view, 9> complexity_nodes = {
        "if_statement",            // if
        "elif_clause",             // elif
        "for_statement",           // for
        "while_statement",         // while
        "try_statement",           // try
        "finally_clause",          // finally
        "case_clause",             // case
        "assert_statement",        // assert
        "conditional_expression",  // для тернарного оператора
    };

    // Подсчитываем количество узлов, увеличивающих цикломатическую сложность
    // используя std::ranges
    auto count_occurrences = [&function_ast](std::string_view node_type) {
        std::string search_pattern = "(" + std::string(node_type);
        size_t count = 0;
        size_t pos = 0;

        while ((pos = function_ast.find(search_pattern, pos)) != std::string::npos) {
            count++;
            pos += search_pattern.length();
        }

        return count;
    };

    // Суммируем количество всех управляющих конструкций
    auto total_complexity = rs::fold_left(
        complexity_nodes | rv::transform(count_occurrences),
        1,  // Базовая сложность функции
        std::plus<>()
    );

    return static_cast<int>(total_complexity);
}
}  // namespace analyzer::metric::metric_impl
