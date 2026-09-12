#include <unistd.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <print>
#include <ranges>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include "file.hpp"
#include "function.hpp"
#include "metric.hpp"
#include "metric_accumulator.hpp"

namespace analyzer {

namespace rv = std::ranges::views;
namespace rs = std::ranges;
/**
 * @brief Анализирует список Python-файлов и извлекает метрики для всех функций и методов.
 *
 * Эта функция — центральный "конвейер" обработки:
 * 1. Принимает имена файлов.
 * 2. Для каждого файла создаёт объект `File`, который автоматически парсит его через tree-sitter
 *    и строит AST.
 * 3. Извлекает из AST все функции и методы с помощью `FunctionExtractor`.
 * 4. Объединяет все функции из всех файлов в один плоский список (`join`).
 * 5. Для каждой функции вычисляет набор метрик через переданный `metric_extractor`.
 * 6. Возвращает вектор пар: (функция, результаты её метрик).
 */
auto AnalyseFunctions(const std::vector<std::string> &files,
                      const analyzer::metric::MetricExtractor &metric_extractor) {
    // 1. Создаём объекты File для каждого файла
    auto file_objects = files
        | rv::transform([](const std::string& filename) {
            return analyzer::file::File(filename);
          });

    // 2. Извлекаем функции из каждого файла и объединяем в один список
    analyzer::function::FunctionExtractor extractor;
    auto all_functions = file_objects
        | rv::transform([&extractor](const auto& file) {
            return extractor.Get(file);
          })
        | rv::join;

    // 3. Для каждой функции вычисляем метрики и формируем пары (функция, метрики)
    auto analysis_results = all_functions
        | rv::transform([&metric_extractor](const auto& func) {
            return std::make_pair(func, metric_extractor.Get(func));
          })
        | rs::to<std::vector>();

    return analysis_results;
}

/**
 *
 * @brief Группирует результаты анализа по классам.
 *
 * Эта функция:
 * 1. Отфильтровывает только те функции, которые являются **методами классов**
 *    (у них `class_name.has_value()` == true).
 * 2. Группирует последовательные элементы с одинаковым именем класса с помощью `chunk_by`.
 *
 * Важно:
 * - `chunk_by` работает только с **последовательными** одинаковыми элементами!
 *   Поэтому предполагается, что входной диапазон уже упорядочен по классам
 *   (например, порядок методов в AST сохраняется как в исходном файле).
 * - Если порядок нарушен, один и тот же класс может быть разбит на несколько групп.
 *
 *  Чтобы убедиться, что фильтрация работает, проверьте, что свободные функции (без class_name)
 * действительно исчезают из результата.
 */
auto SplitByClasses(const auto &analysis) {
    // 1. Фильтруем только методы классов (у которых есть class_name)
    auto class_methods = analysis
        | rv::filter([](const auto& elem) {
            return elem.first.class_name.has_value();
          });

    // 2. Группируем по имени класса с помощью chunk_by
    auto grouped_by_class = class_methods
        | rv::chunk_by([](const auto& a, const auto& b) {
            return a.first.class_name == b.first.class_name;
          });

    return grouped_by_class;
}

/**
 * @brief Группирует результаты анализа по исходным файлам.
 *
 * Эта функция:
 * - Разбивает весь список функций на группы, где каждая группа содержит
 *   только функции из одного и того же файла (`filename`).
 * - Использует `chunk_by`, поэтому **порядок функций в `analysis` должен быть по файлам**.
 */
auto SplitByFiles(const auto &analysis) {
    // Группируем по имени файла с помощью chunk_by
    auto grouped_by_file = analysis
        | rv::chunk_by([](const auto& a, const auto& b) {
            return a.first.filename == b.first.filename;
          });

    return grouped_by_file;
}

/**
 * @brief Агрегирует метрики всех функций с помощью аккумулятора.
 *
 * Эта функция:
 * - Проходит по каждому элементу результата `AnalyseFunctions`
 *   (то есть по каждой функции и её метрикам).
 * - Передаёт результаты метрик (`elem.second`) в аккумулятор через `AccumulateNextFunctionResults`.
 */
void AccumulateFunctionAnalysis(const auto &analysis,
                                const analyzer::metric_accumulator::MetricsAccumulator &accumulator) {
    // Проходим по каждому элементу анализа и передаём метрики в аккумулятор
    rs::for_each(analysis, [&accumulator](const auto& elem) {
        accumulator.AccumulateNextFunctionResults(elem.second);
    });
}

}  // namespace analyzer
