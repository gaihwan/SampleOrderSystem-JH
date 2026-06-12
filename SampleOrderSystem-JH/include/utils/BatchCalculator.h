#pragma once
#include <cmath>

namespace utils {

struct BatchResult {
    int    required_batches = 0;
    double estimated_yield  = 0.0;
};

[[nodiscard]] inline BatchResult CalculateBatch(int quantity, int batch_size, double yield_rate) noexcept {
    const int batches = static_cast<int>(
        std::ceil(static_cast<double>(quantity) / (batch_size * yield_rate))
    );
    return BatchResult{ batches, batches * batch_size * yield_rate };
}

}  // namespace utils
