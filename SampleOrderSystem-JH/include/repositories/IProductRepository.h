#pragma once
#include <optional>
#include <vector>
#include "models/Product.h"

namespace repositories {

class IProductRepository {
public:
    virtual ~IProductRepository() = default;
    [[nodiscard]] virtual int                             Save(const models::Product& product) = 0;
    [[nodiscard]] virtual std::optional<models::Product> FindById(int id) const noexcept = 0;
    [[nodiscard]] virtual std::vector<models::Product>   FindAll() const noexcept = 0;
    [[nodiscard]] virtual bool                           Update(const models::Product& product) noexcept = 0;
};

}  // namespace repositories
