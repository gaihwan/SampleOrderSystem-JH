#pragma once
#include <ostream>
#include <vector>
#include "models/Order.h"
#include "models/Product.h"

namespace views {

class ProductionView {
public:
    explicit ProductionView(std::ostream& output);

    void RenderProductionStatus(const std::vector<models::Order>&   orders,
                                const std::vector<models::Product>& products) const;

private:
    std::ostream& output_;
};

// --- inline implementations ---
inline ProductionView::ProductionView(std::ostream& output) : output_(output) {}

inline void ProductionView::RenderProductionStatus(
        const std::vector<models::Order>&   orders,
        const std::vector<models::Product>& /*products*/) const {
    output_ << "생산 현황\n";
    for (const auto& o : orders) {
        output_ << "주문ID:" << o.id
                << "  필요배치:" << o.required_batches
                << "  예상수율:" << o.estimated_yield << "\n";
    }
}

}  // namespace views
