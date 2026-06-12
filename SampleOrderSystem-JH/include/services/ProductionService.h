#pragma once
#include <vector>
#include "models/Order.h"
#include "repositories/IOrderRepository.h"
#include "services/OrderService.h"   // ServiceResult 재사용

namespace services {

class ProductionService {
public:
    explicit ProductionService(repositories::IOrderRepository& order_repo);

    [[nodiscard]] ServiceResult              StartProduction(int order_id) noexcept;
    [[nodiscard]] ServiceResult              Release(int order_id) noexcept;
    [[nodiscard]] std::vector<models::Order> GetProductionStatus() const noexcept;

private:
    repositories::IOrderRepository& order_repo_;
};

// --- implementations ---
inline ProductionService::ProductionService(repositories::IOrderRepository& order_repo)
    : order_repo_(order_repo) {}

inline ServiceResult ProductionService::StartProduction(int order_id) noexcept {
    // 1. 주문 존재 확인
    auto order = order_repo_.FindById(order_id);
    if (!order.has_value())
        return ServiceResult{false, "주문을 찾을 수 없습니다"};

    // 2. CONFIRMED 상태 확인
    if (order->status != models::OrderStatus::CONFIRMED)
        return ServiceResult{false, "생산 시작할 수 없는 상태입니다"};

    // 3. FIFO: CONFIRMED 주문 중 최솟값 id 탐색
    auto confirmed = order_repo_.FindByStatus(models::OrderStatus::CONFIRMED);
    int min_id = order_id;
    for (const auto& o : confirmed) {
        if (o.id < min_id)
            min_id = o.id;
    }
    if (order_id != min_id)
        return ServiceResult{false, "FIFO 순서 위반: 먼저 생성된 주문을 먼저 생산해야 합니다"};

    // 4. 상태 변경 및 저장
    order->status = models::OrderStatus::PRODUCING;
    if (!order_repo_.Update(*order))
        return ServiceResult{false, "주문 저장에 실패했습니다"};

    return ServiceResult{true, "", order_id};
}

inline ServiceResult ProductionService::Release(int order_id) noexcept {
    // 1. 주문 존재 확인
    auto order = order_repo_.FindById(order_id);
    if (!order.has_value())
        return ServiceResult{false, "주문을 찾을 수 없습니다"};

    // 2. PRODUCING 상태 확인
    if (order->status != models::OrderStatus::PRODUCING)
        return ServiceResult{false, "릴리즈할 수 없는 상태입니다"};

    // 3. 상태 변경 및 저장
    order->status = models::OrderStatus::RELEASE;
    if (!order_repo_.Update(*order))
        return ServiceResult{false, "주문 저장에 실패했습니다"};

    return ServiceResult{true, "", order_id};
}

inline std::vector<models::Order> ProductionService::GetProductionStatus() const noexcept {
    auto confirmed = order_repo_.FindByStatus(models::OrderStatus::CONFIRMED);
    auto producing = order_repo_.FindByStatus(models::OrderStatus::PRODUCING);

    std::vector<models::Order> result;
    result.insert(result.end(), confirmed.begin(), confirmed.end());
    result.insert(result.end(), producing.begin(), producing.end());
    return result;
}

}  // namespace services
