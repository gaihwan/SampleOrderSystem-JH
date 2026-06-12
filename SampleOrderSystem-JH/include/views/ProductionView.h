#pragma once
#include <ostream>
#include <vector>
#include "models/Order.h"
#include "models/Product.h"

namespace views {

class ProductionView {
public:
    explicit ProductionView(std::ostream& output);

    // products: 향후 제품명 표시 등에 활용 예정 (현재 미사용)
    void RenderProductionStatus(const std::vector<models::Order>&   orders,
                                const std::vector<models::Product>& products) const;

private:
    std::ostream& output_;
};

}  // namespace views
