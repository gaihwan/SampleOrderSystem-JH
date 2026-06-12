#pragma once
#include <string>
#include <optional>
#include <ctime>
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

struct [[nodiscard]] ServiceResult {
    bool        success       = false;
    std::string error_message;
    int         order_id      = 0;
};

class OrderService {
public:
    OrderService(repositories::IOrderRepository&  order_repo,
                 repositories::IProductRepository& product_repo);

    // Note: Save() may throw on allocation failure; all other operations are noexcept.
    [[nodiscard]] ServiceResult CreateOrder(const CreateOrderRequest& req);
    [[nodiscard]] ServiceResult ConfirmOrder(int order_id) noexcept;
    [[nodiscard]] ServiceResult RejectOrder(int order_id) noexcept;
    [[nodiscard]] ServiceResult CancelOrder(int order_id) noexcept;

private:
    repositories::IOrderRepository&   order_repo_;
    repositories::IProductRepository& product_repo_;

    // Helper: find order and return nullopt with error if not found
    std::optional<models::Order> FindOrder(int order_id, ServiceResult& err) const noexcept;
};

inline OrderService::OrderService(repositories::IOrderRepository&  order_repo,
                                   repositories::IProductRepository& product_repo)
    : order_repo_(order_repo), product_repo_(product_repo) {}

// TODO: Move implementation to OrderService.cpp when project transitions to production build.
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

    // 주문 생성일 설정 (YYYY-MM-DD)
    {
        std::time_t now = std::time(nullptr);
        struct tm tm_buf = {};
#ifdef _WIN32
        localtime_s(&tm_buf, &now);
#else
        localtime_r(&now, &tm_buf);
#endif
        char date_buf[11];
        std::strftime(date_buf, sizeof(date_buf), "%Y-%m-%d", &tm_buf);
        order.created_at = date_buf;
    }

    // 6. 저장 후 id 반환
    int saved_id = order_repo_.Save(order);
    return ServiceResult{true, "", saved_id};
}

inline std::optional<models::Order> OrderService::FindOrder(int order_id, ServiceResult& err) const noexcept {
    auto order = order_repo_.FindById(order_id);
    if (!order.has_value()) {
        err = ServiceResult{false, "주문을 찾을 수 없습니다"};
        return std::nullopt;
    }
    return order;
}

inline ServiceResult OrderService::ConfirmOrder(int order_id) noexcept {
    ServiceResult err;
    auto order = FindOrder(order_id, err);
    if (!order) return err;
    if (order->status != models::OrderStatus::RESERVED)
        return ServiceResult{false, "확정할 수 없는 상태입니다"};
    if (order_repo_.FindByStatus(models::OrderStatus::CONFIRMED).size() >= 2)
        return ServiceResult{false, "확정 대기 중인 주문이 너무 많습니다"};
    order->status = models::OrderStatus::CONFIRMED;
    if (!order_repo_.Update(*order))
        return ServiceResult{false, "주문 저장에 실패했습니다"};
    return ServiceResult{true, "", order_id};
}

inline ServiceResult OrderService::RejectOrder(int order_id) noexcept {
    ServiceResult err;
    auto order = FindOrder(order_id, err);
    if (!order) return err;
    if (order->status != models::OrderStatus::RESERVED &&
        order->status != models::OrderStatus::CONFIRMED)
        return ServiceResult{false, "반려할 수 없는 상태입니다"};
    order->status = models::OrderStatus::REJECTED;
    if (!order_repo_.Update(*order))
        return ServiceResult{false, "주문 저장에 실패했습니다"};
    return ServiceResult{true, "", order_id};
}

inline ServiceResult OrderService::CancelOrder(int order_id) noexcept {
    ServiceResult err;
    auto order = FindOrder(order_id, err);
    if (!order) return err;
    if (order->status == models::OrderStatus::REJECTED)
        return ServiceResult{false, "이미 거절된 주문입니다"};
    order->status = models::OrderStatus::REJECTED;
    if (!order_repo_.Update(*order))
        return ServiceResult{false, "주문 저장에 실패했습니다"};
    return ServiceResult{true, "", order_id};
}

}  // namespace services
