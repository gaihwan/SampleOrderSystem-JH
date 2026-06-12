#pragma once
#include <algorithm>
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

    // Helper: find order and return nullopt with error if not found
    std::optional<models::Order> FindOrder(int order_id, ServiceResult& err) const noexcept;
};

// --- implementations ---
inline ProductionService::ProductionService(repositories::IOrderRepository& order_repo)
    : order_repo_(order_repo) {}

inline std::optional<models::Order> ProductionService::FindOrder(int order_id, ServiceResult& err) const noexcept {
    auto order = order_repo_.FindById(order_id);
    if (!order.has_value()) {
        err = ServiceResult{false, "주문을 찾을 수 없습니다"};
        return std::nullopt;
    }
    return order;
}

// TODO: Move implementation to ProductionService.cpp when project transitions to production build.
inline ServiceResult ProductionService::StartProduction(int order_id) noexcept {
    // 1. 주문 존재 확인
    ServiceResult err;
    auto order = FindOrder(order_id, err);
    if (!order) return err;

    // 2. CONFIRMED 상태 확인
    if (order->status != models::OrderStatus::CONFIRMED)
        return ServiceResult{false, "생산 시작할 수 없는 상태입니다"};

    // 3. FIFO: CONFIRMED 주문 중 최솟값 id 탐색
    auto confirmed = order_repo_.FindByStatus(models::OrderStatus::CONFIRMED);
    auto it = std::min_element(confirmed.begin(), confirmed.end(),
        [](const models::Order& a, const models::Order& b) { return a.id < b.id; });
    int min_id = it->id;  // confirmed가 비어있지 않음은 앞선 status 검사로 보장
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
    ServiceResult err;
    auto order = FindOrder(order_id, err);
    if (!order) return err;

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
    confirmed.insert(confirmed.end(), producing.begin(), producing.end());
    // Sort by id to ensure consistent FIFO order
    std::sort(confirmed.begin(), confirmed.end(),
        [](const models::Order& a, const models::Order& b) { return a.id < b.id; });
    return confirmed;
}

}  // namespace services
