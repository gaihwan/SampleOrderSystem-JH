#pragma once
#include <unordered_map>
#include "repositories/IProductRepository.h"

namespace repositories {

// Not thread-safe. Intended for single-threaded console use only.
class InMemoryProductRepository : public IProductRepository {
public:
    int                            Save(const models::Product& product) override;
    std::optional<models::Product> FindById(int id) const noexcept override;
    std::vector<models::Product>   FindAll() const noexcept override;
    bool                           Update(const models::Product& product) noexcept override;
private:
    std::unordered_map<int, models::Product> store_;
    int next_id_ = 1;
};

// --- GREEN implementations ---
inline int InMemoryProductRepository::Save(const models::Product& product) {
    models::Product stored = product;
    stored.id = next_id_++;
    store_[stored.id] = stored;
    return stored.id;
}
inline std::optional<models::Product> InMemoryProductRepository::FindById(int id) const noexcept {
    auto it = store_.find(id);
    if (it == store_.end()) return std::nullopt;
    return it->second;
}
inline std::vector<models::Product> InMemoryProductRepository::FindAll() const noexcept {
    std::vector<models::Product> result;
    result.reserve(store_.size());
    for (const auto& [key, product] : store_) result.push_back(product);
    return result;
}
inline bool InMemoryProductRepository::Update(const models::Product& product) noexcept {
    if (store_.find(product.id) == store_.end()) return false;
    store_[product.id] = product;
    return true;
}

}  // namespace repositories
