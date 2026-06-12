#pragma once
#include <unordered_map>
#include "repositories/IOrderRepository.h"

namespace repositories {

class InMemoryOrderRepository : public IOrderRepository {
public:
    int                          Save(const models::Order& order) override;
    std::optional<models::Order> FindById(int id) const override;
    std::vector<models::Order>   FindAll() const override;
    std::vector<models::Order>   FindByStatus(models::OrderStatus status) const override;
    std::vector<models::Order>   FindByProductId(int product_id) const override;
    bool                         Update(const models::Order& order) override;
private:
    std::unordered_map<int, models::Order> store_;
    int next_id_ = 1;
};

// --- GREEN implementations ---
inline int InMemoryOrderRepository::Save(const models::Order& order) {
    models::Order stored = order;
    stored.id = next_id_++;
    store_[stored.id] = stored;
    return stored.id;
}

inline std::optional<models::Order> InMemoryOrderRepository::FindById(int id) const {
    auto it = store_.find(id);
    if (it == store_.end()) return std::nullopt;
    return it->second;
}

inline std::vector<models::Order> InMemoryOrderRepository::FindAll() const {
    std::vector<models::Order> result;
    result.reserve(store_.size());
    for (const auto& [_, order] : store_) result.push_back(order);
    return result;
}

inline std::vector<models::Order> InMemoryOrderRepository::FindByStatus(models::OrderStatus status) const {
    std::vector<models::Order> result;
    for (const auto& [_, order] : store_)
        if (order.status == status) result.push_back(order);
    return result;
}

inline std::vector<models::Order> InMemoryOrderRepository::FindByProductId(int product_id) const {
    std::vector<models::Order> result;
    for (const auto& [_, order] : store_)
        if (order.product_id == product_id) result.push_back(order);
    return result;
}

inline bool InMemoryOrderRepository::Update(const models::Order& order) {
    if (store_.find(order.id) == store_.end()) return false;
    store_[order.id] = order;
    return true;
}

}  // namespace repositories
