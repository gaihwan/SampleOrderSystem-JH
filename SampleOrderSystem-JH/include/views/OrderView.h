#pragma once
#include <ostream>
#include <vector>
#include <string>
#include "models/Order.h"

namespace views {

class OrderView {
public:
    explicit OrderView(std::ostream& output);

    void RenderOrderList(const std::vector<models::Order>& orders,
                         bool show_rejected = true) const;

private:
    std::ostream& output_;
    static std::string StatusToString(models::OrderStatus status);
};

// --- inline implementations ---
inline OrderView::OrderView(std::ostream& output) : output_(output) {}

inline void OrderView::RenderOrderList(const std::vector<models::Order>& orders,
                                        bool show_rejected) const {
    output_ << "ID  수량  마감일      필요배치  상태\n";
    for (const auto& o : orders) {
        if (!show_rejected && o.status == models::OrderStatus::REJECTED) continue;
        output_ << o.id << "  " << o.quantity << "  " << o.deadline
                << "  " << o.required_batches << "  "
                << StatusToString(o.status) << "\n";
    }
}

inline std::string OrderView::StatusToString(models::OrderStatus status) {
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
