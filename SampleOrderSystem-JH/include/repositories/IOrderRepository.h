#pragma once
#include <optional>
#include <vector>
#include "models/Order.h"

namespace repositories {

class IOrderRepository {
public:
    virtual ~IOrderRepository() = default;

    virtual int                          Save(const models::Order& order) = 0;
    virtual std::optional<models::Order> FindById(int id) const = 0;
    virtual std::vector<models::Order>   FindAll() const = 0;
    virtual std::vector<models::Order>   FindByStatus(models::OrderStatus status) const = 0;
    virtual std::vector<models::Order>   FindByProductId(int product_id) const = 0;
    virtual bool                         Update(const models::Order& order) = 0;
};

}  // namespace repositories
