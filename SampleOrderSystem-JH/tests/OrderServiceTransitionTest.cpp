#include <gtest/gtest.h>
#include "services/OrderService.h"
#include "repositories/InMemoryOrderRepository.h"
#include "repositories/InMemoryProductRepository.h"

class OrderServiceTransitionTest : public ::testing::Test {
protected:
    repositories::InMemoryOrderRepository   order_repo_;
    repositories::InMemoryProductRepository product_repo_;
    services::OrderService service_{order_repo_, product_repo_};

    // 헬퍼: product 저장 후 RESERVED 주문 생성, order_id 반환
    int CreateReservedOrder() {
        models::Product p;
        p.name       = "TestSample";
        p.batch_size = 50;
        p.batch_days = 7;
        p.yield_rate = 0.9;
        int pid = product_repo_.Save(p);

        auto result = service_.CreateOrder({pid, 100, "2099-12-31"});
        return result.order_id;
    }
};

// TC-01: RESERVED → CONFIRMED
TEST_F(OrderServiceTransitionTest, ConfirmOrder_ChangesStatus_ToConfirmed) {
    int oid = CreateReservedOrder();
    auto result = service_.ConfirmOrder(oid);
    ASSERT_TRUE(result.success);
    auto order = order_repo_.FindById(oid);
    ASSERT_TRUE(order.has_value());
    EXPECT_EQ(order->status, models::OrderStatus::CONFIRMED);
}

// TC-02: CONFIRMED 상태에서 ConfirmOrder 실패
TEST_F(OrderServiceTransitionTest, ConfirmOrder_Fails_WhenStatusIsNotReserved) {
    int oid = CreateReservedOrder();
    (void)service_.ConfirmOrder(oid);  // 첫 번째 confirm
    auto result = service_.ConfirmOrder(oid);  // 두 번째 confirm → 실패
    EXPECT_FALSE(result.success);
}

// TC-03: RESERVED → REJECTED
TEST_F(OrderServiceTransitionTest, RejectOrder_ChangesStatus_ToRejected_FromReserved) {
    int oid = CreateReservedOrder();
    auto result = service_.RejectOrder(oid);
    ASSERT_TRUE(result.success);
    auto order = order_repo_.FindById(oid);
    ASSERT_TRUE(order.has_value());
    EXPECT_EQ(order->status, models::OrderStatus::REJECTED);
}

// TC-04: CONFIRMED → REJECTED
TEST_F(OrderServiceTransitionTest, RejectOrder_ChangesStatus_ToRejected_FromConfirmed) {
    int oid = CreateReservedOrder();
    (void)service_.ConfirmOrder(oid);
    auto result = service_.RejectOrder(oid);
    ASSERT_TRUE(result.success);
    auto order = order_repo_.FindById(oid);
    ASSERT_TRUE(order.has_value());
    EXPECT_EQ(order->status, models::OrderStatus::REJECTED);
}

// TC-05: 비REJECTED 상태 → REJECTED (cancel)
TEST_F(OrderServiceTransitionTest, CancelOrder_ChangesStatus_ToRejected_FromAnyNonRejectedStatus) {
    int oid = CreateReservedOrder();
    auto result = service_.CancelOrder(oid);
    ASSERT_TRUE(result.success);
    auto order = order_repo_.FindById(oid);
    ASSERT_TRUE(order.has_value());
    EXPECT_EQ(order->status, models::OrderStatus::REJECTED);
}

// TC-06: 이미 REJECTED 상태에서 CancelOrder 실패
TEST_F(OrderServiceTransitionTest, CancelOrder_Fails_WhenAlreadyRejected) {
    int oid = CreateReservedOrder();
    (void)service_.RejectOrder(oid);
    auto result = service_.CancelOrder(oid);
    EXPECT_FALSE(result.success);
}

// TC-07: 존재하지 않는 ID로 CancelOrder 실패
TEST_F(OrderServiceTransitionTest, CancelOrder_Fails_WhenOrderNotFound) {
    auto result = service_.CancelOrder(999);
    EXPECT_FALSE(result.success);
}
