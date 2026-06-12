#pragma once
#include <optional>
#include <vector>
#include "models/Order.h"

namespace repositories {

class IOrderRepository {
public:
    virtual ~IOrderRepository() = default;

    [[nodiscard]] virtual int                          Save(const models::Order& order) = 0;
    [[nodiscard]] virtual std::optional<models::Order> FindById(int id) const noexcept = 0;
    [[nodiscard]] virtual std::vector<models::Order>   FindAll() const noexcept = 0;
    [[nodiscard]] virtual std::vector<models::Order>   FindByStatus(models::OrderStatus status) const noexcept = 0;
    [[nodiscard]] virtual std::vector<models::Order>   FindByProductId(int product_id) const noexcept = 0;
    [[nodiscard]] virtual bool                         Update(const models::Order& order) noexcept = 0;
};

}  // namespace repositories
