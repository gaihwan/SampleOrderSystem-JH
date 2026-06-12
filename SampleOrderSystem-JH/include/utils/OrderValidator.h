#pragma once
#include <cctype>
#include <string>
#include <unordered_set>

namespace utils {

struct ValidationResult {
    bool        is_valid = true;
    std::string error_message;
};

class OrderValidator {
public:
    [[nodiscard]] static ValidationResult ValidateQuantity(int quantity) noexcept;
    [[nodiscard]] static ValidationResult ValidateDeadline(const std::string& deadline) noexcept;
    [[nodiscard]] static ValidationResult ValidateProductId(
        int product_id,
        const std::unordered_set<int>& valid_ids) noexcept;
};

inline ValidationResult OrderValidator::ValidateQuantity(int quantity) noexcept {
    if (quantity < 1) {
        return ValidationResult{false, "수량은 1 이상이어야 합니다"};
    }
    return ValidationResult{};
}

inline ValidationResult OrderValidator::ValidateDeadline(const std::string& deadline) noexcept {
    // YYYY-MM-DD: 길이 10, 위치 4·7 에 '-', 나머지는 숫자
    if (deadline.size() != 10 ||
        deadline[4] != '-' || deadline[7] != '-') {
        return ValidationResult{false, "날짜 형식이 올바르지 않습니다 (YYYY-MM-DD)"};
    }
    for (int i : {0, 1, 2, 3, 5, 6, 8, 9}) {
        if (!std::isdigit(static_cast<unsigned char>(deadline[i]))) {
            return ValidationResult{false, "날짜 형식이 올바르지 않습니다 (YYYY-MM-DD)"};
        }
    }
    return ValidationResult{};
}

inline ValidationResult OrderValidator::ValidateProductId(
    int product_id,
    const std::unordered_set<int>& valid_ids) noexcept {
    if (valid_ids.find(product_id) == valid_ids.end()) {
        return ValidationResult{false, "시료를 찾을 수 없습니다"};
    }
    return ValidationResult{};
}

}  // namespace utils