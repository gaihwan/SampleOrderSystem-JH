#pragma once
#include <string>
#include "repositories/IOrderRepository.h"
#include "repositories/IProductRepository.h"
#include "utils/BatchCalculator.h"
#include "utils/OrderValidator.h"
#include "models/Order.h"

namespace services {

struct CreateOrderRequest {
    int         product_id = 0;
    int         quantity   = 0;
    std::string deadline;
};

struct ServiceResult {
    bool        success       = false;
    std::string error_message;
    int         order_id      = 0;
};

class OrderService {
public:
    OrderService(repositories::IOrderRepository&  order_repo,
                 repositories::IProductRepository& product_repo);

    [[nodiscard]] ServiceResult CreateOrder(const CreateOrderRequest& req);

private:
    repositories::IOrderRepository&  order_repo_;
    repositories::IProductRepository& product_repo_;
};

inline OrderService::OrderService(repositories::IOrderRepository&  order_repo,
                                   repositories::IProductRepository& product_repo)
    : order_repo_(order_repo), product_repo_(product_repo) {}

inline ServiceResult OrderService::CreateOrder(const CreateOrderRequest& req) {
    // 1. 수량 유효성 검사
    auto qty_result = utils::OrderValidator::ValidateQuantity(req.quantity);
    if (!qty_result.is_valid) {
        return ServiceResult{false, qty_result.error_message};
    }

    // 2. 납기일 유효성 검사
    auto dl_result = utils::OrderValidator::ValidateDeadline(req.deadline);
    if (!dl_result.is_valid) {
        return ServiceResult{false, dl_result.error_message};
    }

    // 3. 시료 존재 여부 확인
    auto product = product_repo_.FindById(req.product_id);
    if (!product.has_value()) {
        return ServiceResult{false, "시료를 찾을 수 없습니다"};
    }

    // 4. 배치 자동 계산
    auto batch_result = utils::CalculateBatch(
        req.quantity, product->batch_size, product->yield_rate);

    // 5. Order 구성
    models::Order order;
    order.product_id       = req.product_id;
    order.quantity         = req.quantity;
    order.deadline         = req.deadline;
    order.status           = models::OrderStatus::RESERVED;
    order.required_batches = batch_result.required_batches;
    order.estimated_yield  = batch_result.estimated_yield;

    // 6. 저장 후 id 반환
    int saved_id = order_repo_.Save(order);
    return ServiceResult{true, "", saved_id};
}

}  // namespace services
