#pragma once
#include <optional>
#include <vector>
#include "models/Product.h"

namespace repositories {

class IProductRepository {
public:
    virtual ~IProductRepository() = default;
    [[nodiscard]] virtual int                             Save(const models::Product& product) = 0;
    [[nodiscard]] virtual std::optional<models::Product> FindById(int id) const = 0;
    [[nodiscard]] virtual std::vector<models::Product>   FindAll() const = 0;
    virtual bool                                         Update(const models::Product& product) = 0;
};

}  // namespace repositories
