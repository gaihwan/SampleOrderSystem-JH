#include <gtest/gtest.h>
#include "services/ProductionService.h"
#include "services/OrderService.h"
#include "repositories/InMemoryOrderRepository.h"
#include "repositories/InMemoryProductRepository.h"

class ProductionServiceTest : public ::testing::Test {
protected:
    repositories::InMemoryOrderRepository   order_repo_;
    repositories::InMemoryProductRepository product_repo_;
    services::OrderService      order_service_{order_repo_, product_repo_};
    services::ProductionService production_service_{order_repo_};

    // Helper: product 저장 후 CONFIRMED 주문 생성, order_id 반환
    int CreateConfirmedOrder() {
        models::Product p;
        p.name       = "TestSample";
        p.batch_size = 50;
        p.batch_days = 7;
        p.yield_rate = 0.9;
        int pid = product_repo_.Save(p);
        auto r = order_service_.CreateOrder({pid, 100, "2099-12-31"});
        (void)order_service_.ConfirmOrder(r.order_id);
        return r.order_id;
    }
};

// TC-01: CONFIRMED -> PRODUCING
TEST_F(ProductionServiceTest, StartProduction_ChangesStatus_ToProducing) {
    int oid = CreateConfirmedOrder();
    auto result = production_service_.StartProduction(oid);
    ASSERT_TRUE(result.success);
    auto order = order_repo_.FindById(oid);
    ASSERT_TRUE(order.has_value());
    EXPECT_EQ(order->status, models::OrderStatus::PRODUCING);
}

// TC-02: 비CONFIRMED 상태(RESERVED) -> 실패
TEST_F(ProductionServiceTest, StartProduction_Fails_WhenStatusIsNotConfirmed) {
    models::Product p;
    p.name = "TestSample"; p.batch_size = 50; p.batch_days = 7; p.yield_rate = 0.9;
    int pid = product_repo_.Save(p);
    auto r = order_service_.CreateOrder({pid, 100, "2099-12-31"});
    // RESERVED 상태 그대로 (ConfirmOrder 호출 안 함)
    auto result = production_service_.StartProduction(r.order_id);
    EXPECT_FALSE(result.success);
}

// TC-03: FIFO 순서 강제 -- id=2 먼저 시도 -> 실패
TEST_F(ProductionServiceTest, StartProduction_FollowsFIFO_Order) {
    int oid1 = CreateConfirmedOrder();  // 더 먼저 생성
    int oid2 = CreateConfirmedOrder();
    (void)oid1;
    // oid2를 먼저 생산 시도 -> FIFO 위반 -> 실패
    auto result = production_service_.StartProduction(oid2);
    EXPECT_FALSE(result.success);
}

// TC-04: PRODUCING -> RELEASE
TEST_F(ProductionServiceTest, Release_ChangesStatus_ToRelease) {
    int oid = CreateConfirmedOrder();
    (void)production_service_.StartProduction(oid);
    auto result = production_service_.Release(oid);
    ASSERT_TRUE(result.success);
    auto order = order_repo_.FindById(oid);
    ASSERT_TRUE(order.has_value());
    EXPECT_EQ(order->status, models::OrderStatus::RELEASE);
}

// TC-05: 비PRODUCING 상태(CONFIRMED) -> Release 실패
TEST_F(ProductionServiceTest, Release_Fails_WhenStatusIsNotProducing) {
    int oid = CreateConfirmedOrder();
    auto result = production_service_.Release(oid);
    EXPECT_FALSE(result.success);
}

// TC-06: GetProductionStatus -- CONFIRMED + PRODUCING 반환, RESERVED 제외
TEST_F(ProductionServiceTest, GetProductionStatus_ReturnsConfirmedAndProducingOrders) {
    int oid1 = CreateConfirmedOrder();  // CONFIRMED
    int oid2 = CreateConfirmedOrder();  // CONFIRMED (oid1 이후 생성)
    (void)production_service_.StartProduction(oid1);  // oid1이 FIFO상 먼저 -> PRODUCING
    // RESERVED 주문 1개 추가
    models::Product p;
    p.name = "TestSample"; p.batch_size = 50; p.batch_days = 7; p.yield_rate = 0.9;
    int pid = product_repo_.Save(p);
    (void)order_service_.CreateOrder({pid, 100, "2099-12-31"});  // RESERVED
    (void)oid2;

    auto statuses = production_service_.GetProductionStatus();
    EXPECT_EQ(statuses.size(), 2u);  // PRODUCING(oid1) + CONFIRMED(oid2)
}
