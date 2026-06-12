#pragma once
#include <string>

namespace models {

enum class OrderStatus {
    RESERVED,
    CONFIRMED,
    PRODUCING,
    RELEASE,
    REJECTED
};

struct Order {
    int id = 0;
    int product_id = 0;
    int quantity = 0;
    std::string deadline;
    OrderStatus status = OrderStatus::RESERVED;
    int required_batches = 0;
    double estimated_yield = 0.0;
    std::string created_at;
};

}  // namespace models
