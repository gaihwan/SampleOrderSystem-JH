#include <gtest/gtest.h>
#include "services/OrderService.h"
#include "repositories/InMemoryOrderRepository.h"
#include "repositories/InMemoryProductRepository.h"

class OrderServiceCreateTest : public ::testing::Test {
protected:
    repositories::InMemoryOrderRepository   order_repo_;
    repositories::InMemoryProductRepository product_repo_;
    services::OrderService service_{ order_repo_, product_repo_ };

    // 헬퍼: 유효한 Product를 product_repo_에 저장하고 id 반환
    int SaveProduct(int batch_size = 50, double yield_rate = 0.9) {
        models::Product p;
        p.name       = "TestSample";
        p.batch_size = batch_size;
        p.batch_days = 7;
        p.yield_rate = yield_rate;
        return product_repo_.Save(p);
    }

    // 헬퍼: 유효한 CreateOrderRequest 생성
    services::CreateOrderRequest MakeRequest(int product_id, int quantity = 100,
                                              const std::string& deadline = "2025-12-31") {
        return services::CreateOrderRequest{ product_id, quantity, deadline };
    }
};

// TC-01: 주문 생성 성공 시 RESERVED 상태로 저장된다
TEST_F(OrderServiceCreateTest, CreateOrder_ReturnsOrderWithReservedStatus) {
    int pid = SaveProduct();
    auto req = MakeRequest(pid);

    auto result = service_.CreateOrder(req);

    ASSERT_TRUE(result.success);
    auto stored = order_repo_.FindById(result.order_id);
    ASSERT_TRUE(stored.has_value());
    EXPECT_EQ(stored->status, models::OrderStatus::RESERVED);
}

// TC-02: required_batches 가 자동으로 계산된다 (batch_size=50, yield=0.9, qty=100 → ceil(100/45)=3)
TEST_F(OrderServiceCreateTest, CreateOrder_CalculatesRequiredBatchesAutomatically) {
    int pid = SaveProduct(50, 0.9);
    auto req = MakeRequest(pid, 100);

    auto result = service_.CreateOrder(req);

    ASSERT_TRUE(result.success);
    auto stored = order_repo_.FindById(result.order_id);
    ASSERT_TRUE(stored.has_value());
    EXPECT_EQ(stored->required_batches, 3);
}

// TC-03: estimated_yield 가 자동으로 계산된다 (3 * 50 * 0.9 = 135.0)
TEST_F(OrderServiceCreateTest, CreateOrder_CalculatesEstimatedYieldAutomatically) {
    int pid = SaveProduct(50, 0.9);
    auto req = MakeRequest(pid, 100);

    auto result = service_.CreateOrder(req);

    ASSERT_TRUE(result.success);
    auto stored = order_repo_.FindById(result.order_id);
    ASSERT_TRUE(stored.has_value());
    EXPECT_DOUBLE_EQ(stored->estimated_yield, 135.0);
}

// TC-04: 존재하지 않는 product_id 로 주문 시 실패한다
TEST_F(OrderServiceCreateTest, CreateOrder_Fails_WhenProductNotFound) {
    auto req = MakeRequest(999);  // product_repo_에 저장 안 함

    auto result = service_.CreateOrder(req);

    EXPECT_FALSE(result.success);
}

// TC-05: quantity 가 0 이면 실패한다
TEST_F(OrderServiceCreateTest, CreateOrder_Fails_WhenQuantityIsZero) {
    int pid = SaveProduct();
    auto req = MakeRequest(pid, 0);

    auto result = service_.CreateOrder(req);

    EXPECT_FALSE(result.success);
}

// TC-06: 날짜 형식이 YYYY-MM-DD 가 아니면 실패한다
TEST_F(OrderServiceCreateTest, CreateOrder_Fails_WhenDeadlineFormatInvalid) {
    int pid = SaveProduct();
    auto req = MakeRequest(pid, 100, "20251231");  // 잘못된 형식

    auto result = service_.CreateOrder(req);

    EXPECT_FALSE(result.success);
}
