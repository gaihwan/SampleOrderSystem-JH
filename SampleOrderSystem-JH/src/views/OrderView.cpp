#include "views/OrderView.h"
#include <iomanip>

namespace views {

OrderView::OrderView(std::ostream& output) : output_(output) {}

void OrderView::RenderOrderList(const std::vector<models::Order>& orders,
                                 bool show_rejected) const {
    output_ << std::left
            << std::setw(6)  << "ID"
            << std::setw(8)  << "수량"
            << std::setw(14) << "마감일"
            << std::setw(10) << "필요배치"
            << "상태\n";
    for (const auto& o : orders) {
        if (!show_rejected && o.status == models::OrderStatus::REJECTED) continue;
        output_ << std::left
                << std::setw(6)  << o.id
                << std::setw(8)  << o.quantity
                << std::setw(14) << o.deadline
                << std::setw(10) << o.required_batches
                << StatusToString(o.status) << "\n";
    }
}

std::string OrderView::StatusToString(models::OrderStatus status) {
    switch (status) {
        case models::OrderStatus::RESERVED:  return "RESERVED";
        case models::OrderStatus::CONFIRMED: return "CONFIRMED";
        case models::OrderStatus::PRODUCING: return "PRODUCING";
        case models::OrderStatus::RELEASE:   return "RELEASE";
        case models::OrderStatus::REJECTED:  return "REJECTED";
        default: return "UNKNOWN";
    }
}

}  // namespace views
