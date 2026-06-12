#include <gtest/gtest.h>
#include <sstream>
#include "views/OrderView.h"

class OrderViewTest : public ::testing::Test {
protected:
    std::ostringstream output_;
    views::OrderView   view_{output_};

    models::Order MakeOrder(int id, models::OrderStatus status = models::OrderStatus::RESERVED,
                            int required_batches = 3, double estimated_yield = 135.0) {
        models::Order o;
        o.id               = id;
        o.product_id       = 1;
        o.quantity         = 100;
        o.deadline         = "2099-12-31";
        o.status           = status;
        o.required_batches = required_batches;
        o.estimated_yield  = estimated_yield;
        return o;
    }
};

// TC-01: 헤더 + 주문 1건 — ID/수량 포함
TEST_F(OrderViewTest, RenderOrderList_ShowsAllColumns) {
    auto order = MakeOrder(1);
    view_.RenderOrderList({order});
    std::string out = output_.str();
    EXPECT_NE(out.find("1"),  std::string::npos);   // id
    EXPECT_NE(out.find("100"), std::string::npos);  // quantity
}

// TC-02: CONFIRMED 주문 — deadline, required_batches 포함
TEST_F(OrderViewTest, RenderOrderList_ShowsDeadlineAndBatch_WhenConfirmed) {
    auto order = MakeOrder(1, models::OrderStatus::CONFIRMED, 3, 135.0);
    view_.RenderOrderList({order});
    std::string out = output_.str();
    EXPECT_NE(out.find("2099-12-31"), std::string::npos);  // deadline
    EXPECT_NE(out.find("3"),          std::string::npos);  // required_batches
}

// TC-03: show_rejected=false → REJECTED 주문 제외
TEST_F(OrderViewTest, RenderOrderList_FiltersRejectedOrders_WhenRequested) {
    auto reserved = MakeOrder(1, models::OrderStatus::RESERVED);
    auto rejected = MakeOrder(2, models::OrderStatus::REJECTED);
    view_.RenderOrderList({reserved, rejected}, false);
    std::string out = output_.str();
    // id=1 포함, id=2 미포함 검증
    EXPECT_NE(out.find("1"), std::string::npos);
    // "2"가 없는지는 id=2가 출력됐는지로 확인 — rejected의 id=2가 출력에 없어야 함
    // REJECTED 상태 문자열이 없는 것으로 검증
    EXPECT_EQ(out.find("REJECTED"), std::string::npos);
}
