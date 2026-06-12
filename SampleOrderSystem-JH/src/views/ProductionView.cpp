#include "views/ProductionView.h"
#include <iomanip>

namespace views {

ProductionView::ProductionView(std::ostream& output) : output_(output) {}

void ProductionView::RenderProductionStatus(
        const std::vector<models::Order>&   orders,
        const std::vector<models::Product>& /* products */) const {
    output_ << "생산 현황\n";
    output_ << std::left
            << std::setw(10) << "주문ID"
            << std::setw(12) << "필요배치"
            << "예상수율\n";
    for (const auto& o : orders) {
        output_ << std::left
                << std::setw(10) << o.id
                << std::setw(12) << o.required_batches
                << o.estimated_yield << "\n";
    }
}

}  // namespace views
