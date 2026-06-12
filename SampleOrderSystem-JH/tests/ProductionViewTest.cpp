#include <gtest/gtest.h>
#include <sstream>
#include "views/ProductionView.h"

class ProductionViewTest : public ::testing::Test {
protected:
    std::ostringstream output_;
    views::ProductionView view_{output_};

    models::Order MakeProducingOrder(int id, int required_batches = 3,
                                     double estimated_yield = 135.0) {
        models::Order o;
        o.id               = id;
        o.product_id       = 1;
        o.quantity         = 100;
        o.deadline         = "2099-12-31";
        o.status           = models::OrderStatus::PRODUCING;
        o.required_batches = required_batches;
        o.estimated_yield  = estimated_yield;
        return o;
    }

    models::Product MakeProduct(int id, const std::string& name = "TestSample") {
        models::Product p;
        p.id         = id;
        p.name       = name;
        p.batch_size = 50;
        p.batch_days = 7;
        p.yield_rate = 0.9;
        return p;
    }
};

// TC-04: 생산 현황 — "생산 현황", 배치수 포함
TEST_F(ProductionViewTest, RenderProductionStatus_ShowsBatchProgress) {
    auto order   = MakeProducingOrder(1, 3);
    auto product = MakeProduct(1, "TestSample");
    view_.RenderProductionStatus({order}, {product});
    std::string out = output_.str();
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out.find("3"), std::string::npos);  // required_batches
}

// TC-05: estimated_yield 포함
TEST_F(ProductionViewTest, RenderProductionStatus_ShowsEstimatedCompletionDate) {
    auto order   = MakeProducingOrder(1, 3, 135.0);
    auto product = MakeProduct(1);
    view_.RenderProductionStatus({order}, {product});
    std::string out = output_.str();
    EXPECT_NE(out.find("135"), std::string::npos);  // estimated_yield
}
