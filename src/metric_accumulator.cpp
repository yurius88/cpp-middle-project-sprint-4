#include "metric_accumulator.hpp"

#include <unistd.h>

#include <algorithm>
#include <any>
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
#include <variant>
#include <vector>

namespace analyzer::metric_accumulator {
/**
 * @brief Накапливает результаты метрик для одной функции.
 *
 * Эта функция принимает вектор результатов метрик (например, "цикломатическая сложность = 4",
 * "число строк = 12", "параметров = 3") и передаёт каждый результат соответствующему аккумулятору.
 *
 * Как это работает:
 * - Для каждого `metric_result` из `metric_results` извлекается имя метрики (`metric_name`).
 * - По этому имени в контейнере `accumulators` находится нужный аккумулятор.
 * - Вызывается метод `Accumulate(metric_result)`, который обновляет внутреннее состояние аккумулятора.
 */
void MetricsAccumulator::AccumulateNextFunctionResults(const std::vector<metric::MetricResult> &metric_results) const {
    if (metric_results.empty()) {
        throw std::runtime_error("MetricsAccumulator::AccumulateNextFunctionResults: metric_results vector is empty");
    }

    rs::for_each(metric_results, [this](const auto& result) {
        auto it = accumulators.find(result.metric_name);
        if (it == accumulators.end()) {
            throw std::runtime_error("MetricsAccumulator::AccumulateNextFunctionResults: no accumulator registered for metric '" +
                                   result.metric_name + "'");
        }
        it->second->Accumulate(result);
    });
}
/**
 * @brief Сбрасывает состояние всех аккумуляторов.
 *
 * Эта функция вызывается, когда нужно начать новую агрегацию (например, для нового файла или класса).
 * Она проходит по всем зарегистрированным аккумуляторам и вызывает у каждого метод `Reset()`,
 * который обнуляет накопленные значения (сумму, счётчик и т.д.).
 */
void MetricsAccumulator::ResetAccumulators() {
    if (accumulators.empty()) {
        throw std::runtime_error("MetricsAccumulator::ResetAccumulators: no accumulators registered");
    }

    rs::for_each(accumulators | rv::values, [](auto& accumulator) {
        if (!accumulator) {
            throw std::runtime_error("MetricsAccumulator::ResetAccumulators: null accumulator pointer detected");
        }
        accumulator->Reset();
    });
}

}  // namespace analyzer::metric_accumulator
