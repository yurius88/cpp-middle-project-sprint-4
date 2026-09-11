#include "metric_impl/code_lines_count.hpp"

#include <unistd.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <ranges>
#include <sstream>
#include <string>
#include <utils.hpp>
#include <variant>
#include <vector>

namespace analyzer::metric::metric_impl {
std::string CodeLinesCountMetric::Name() const { return kName; }

MetricResult::ValueType CodeLinesCountMetric::CalculateImpl(const function::Function &f) const {
    auto &function_ast = f.ast;

    if (function_ast.empty()) {
        throw std::runtime_error("CodeLinesCountMetric: empty AST for function " + f.name);
    }

    // Вспомогательная лямбда для извлечения номера строки из диапазона узла AST.
    // Формат узла в S-выражении: (node_type [start_line,start_column] [end_line,end_column] ...)
    // Эта функция ищет открывающую скобку "[" после заданной позиции и парсит первую координату — номер строки.
    auto line_number = [&](int start_pos) {
        size_t line_pos = function_ast.find("[", start_pos);
        if (line_pos == std::string::npos) {
            throw std::runtime_error("CodeLinesCountMetric: malformed AST - missing '[' bracket at position " +
                                   std::to_string(start_pos) + " in function " + f.name);
        }
        size_t comma_pos = function_ast.find(",", line_pos);
        if (comma_pos == std::string::npos) {
            throw std::runtime_error("CodeLinesCountMetric: malformed AST - missing ',' separator after position " +
                                   std::to_string(line_pos) + " in function " + f.name);
        }
        try {
            return ToInt(function_ast.substr(line_pos + 1, comma_pos - line_pos - 1));
        } catch (const std::invalid_argument& e) {
            throw std::runtime_error("CodeLinesCountMetric: failed to parse line number in function " +
                                   f.name + ": " + e.what());
        }
    };
    // Определяем начальную и конечную строки тела функции:
    // - начальная строка берётся из корневого узла функции (первое вхождение "[")
    // - конечная строка ищется по шаблону "] -"
    const int start_line = line_number(0);
    size_t end_marker_pos = function_ast.find("] -");
    if (end_marker_pos == std::string::npos) {
        throw std::runtime_error("CodeLinesCountMetric: malformed AST - missing '] -' marker in function " + f.name);
    }
    const int end_line = line_number(end_marker_pos);

    if (end_line < start_line) {
        throw std::runtime_error("CodeLinesCountMetric: invalid line range [" + std::to_string(start_line) +
                               ", " + std::to_string(end_line) + "] in function " + f.name);
    }

    // Лямбда, проверяющая, является ли конкретная строка "кодовой", то есть не комментарием.
    auto is_code_line = [&](int line) {
        std::string line_marker = "[" + std::to_string(line) + ",";
        size_t line_pos = function_ast.find(line_marker);

        if (line_pos == std::string::npos)
            return false;

        size_t node_start = function_ast.rfind('(', line_pos);
        if (node_start == std::string::npos)
            return false;

        std::string_view node_type =
            std::string_view(function_ast)
                .substr(node_start + 1, function_ast.find_first_of(" \n[", node_start + 1) - node_start - 1);

        return node_type != "comment";
    };
    // Подсчитываем количество строк кода в диапазоне [start_line + 1, end_line]
    // используя std::ranges и views
    // start_line + 1, т.к. первая строка — объявление функции (def ...)
    auto lines_range = rv::iota(start_line + 1, end_line + 1);
    auto code_lines_count = rs::count_if(lines_range, is_code_line);

    return static_cast<int>(code_lines_count);
}

}  // namespace analyzer::metric::metric_impl
