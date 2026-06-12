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
    // TODO: 향후 view 유틸리티로 추출 가능
    static std::string StatusToString(models::OrderStatus status);
};

}  // namespace views
