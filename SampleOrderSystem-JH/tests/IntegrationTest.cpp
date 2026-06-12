#include <gtest/gtest.h>
#include <sstream>
#include <cstdio>
#include "repositories/InMemoryOrderRepository.h"
#include "repositories/InMemoryProductRepository.h"
#include "repositories/FileOrderRepository.h"
#include "services/OrderService.h"
#include "services/ProductionService.h"

// -- 공통 픽스처 ----------------------------------------------------------------
class IntegrationTest : public ::testing::Test {
protected:
    repositories::InMemoryOrderRepository   order_repo_;
    repositories::InMemoryProductRepository product_repo_;

    void SetUp() override {
        models::Product p;
        p.id         = 1;
        p.name       = "SiC-100";
        p.batch_size = 50;
        p.batch_days = 7;
        p.yield_rate = 0.9;
        product_repo_.Save(p);
    }
};

// TC-01: RESERVED -> CONFIRMED -> PRODUCING -> RELEASE 전체 흐름
TEST_F(IntegrationTest, FullLifecycle_ReservedToRelease) {
    services::OrderService      order_svc(order_repo_, product_repo_);
    services::ProductionService prod_svc(order_repo_);

    auto create = order_svc.CreateOrder({1, 100, "2099-12-31"});
    ASSERT_TRUE(create.success);

    auto confirm = order_svc.ConfirmOrder(create.order_id);
    ASSERT_TRUE(confirm.success);

    auto start = prod_svc.StartProduction(create.order_id);
    ASSERT_TRUE(start.success);

    auto release = prod_svc.Release(create.order_id);
    ASSERT_TRUE(release.success);

    auto order = order_repo_.FindById(create.order_id);
    ASSERT_TRUE(order.has_value());
    EXPECT_EQ(order->status, models::OrderStatus::RELEASE);
}

// TC-02: RESERVED -> REJECTED 흐름
TEST_F(IntegrationTest, FullLifecycle_ReservedToRejected) {
    services::OrderService order_svc(order_repo_, product_repo_);

    auto create = order_svc.CreateOrder({1, 100, "2099-12-31"});
    ASSERT_TRUE(create.success);

    auto reject = order_svc.RejectOrder(create.order_id);
    ASSERT_TRUE(reject.success);

    auto order = order_repo_.FindById(create.order_id);
    ASSERT_TRUE(order.has_value());
    EXPECT_EQ(order->status, models::OrderStatus::REJECTED);
}

// TC-03: 배치 계산이 생성된 주문에 반영됨
TEST_F(IntegrationTest, BatchCalculation_ReflectedInCreatedOrder) {
    services::OrderService order_svc(order_repo_, product_repo_);

    // batch_size=50, yield_rate=0.9 -> required_batches = ceil(100/(50*0.9)) = ceil(2.22) = 3
    // estimated_yield = 3 * 50 * 0.9 = 135.0
    auto create = order_svc.CreateOrder({1, 100, "2099-12-31"});
    ASSERT_TRUE(create.success);

    auto order = order_repo_.FindById(create.order_id);
    ASSERT_TRUE(order.has_value());
    EXPECT_EQ(order->required_batches, 3);
    EXPECT_DOUBLE_EQ(order->estimated_yield, 135.0);
}

// TC-04: FileOrderRepository -- 재로드 후 데이터 복원
class IntegrationFileRepoTest : public ::testing::Test {
protected:
    const std::string test_file_ = "integration_test_orders.jsonl";

    void TearDown() override {
        std::remove(test_file_.c_str());
    }
};

TEST_F(IntegrationFileRepoTest, FileRepository_RestoresDataAfterRestart) {
    repositories::InMemoryProductRepository product_repo;
    models::Product p;
    p.id = 1; p.name = "SiC-100"; p.batch_size = 50; p.batch_days = 7; p.yield_rate = 0.9;
    product_repo.Save(p);

    // 저장
    {
        repositories::FileOrderRepository file_repo(test_file_);
        services::OrderService svc(file_repo, product_repo);
        auto r = svc.CreateOrder({1, 100, "2099-12-31"});
        ASSERT_TRUE(r.success);
    }

    // 재로드 후 조회
    {
        repositories::FileOrderRepository file_repo(test_file_);
        auto all = file_repo.FindAll();
        ASSERT_EQ(all.size(), 1u);
        EXPECT_EQ(all[0].product_id, 1);
        EXPECT_EQ(all[0].quantity, 100);
        EXPECT_EQ(all[0].status, models::OrderStatus::RESERVED);
    }
}

// TC-05: FIFO -- 먼저 CONFIRMED된 주문이 먼저 StartProduction 가능
TEST_F(IntegrationTest, FifoProduction_ConfirmedFirstIsProducedFirst) {
    services::OrderService      order_svc(order_repo_, product_repo_);
    services::ProductionService prod_svc(order_repo_);

    auto r1 = order_svc.CreateOrder({1, 100, "2099-12-31"});
    auto r2 = order_svc.CreateOrder({1, 100, "2099-12-31"});
    ASSERT_TRUE(r1.success);
    ASSERT_TRUE(r2.success);

    order_svc.ConfirmOrder(r1.order_id);
    order_svc.ConfirmOrder(r2.order_id);

    // r1이 먼저 CONFIRMED -> StartProduction 성공
    auto start1 = prod_svc.StartProduction(r1.order_id);
    EXPECT_TRUE(start1.success);

    // r1이 PRODUCING으로 전환된 후 r2가 최소 id CONFIRMED -> StartProduction 성공
    auto start2 = prod_svc.StartProduction(r2.order_id);
    EXPECT_TRUE(start2.success);
}

// TC-06: CONFIRMED가 2개 이상이면 추가 확정 불가 (SPEC 3.3절)
TEST_F(IntegrationTest, ConfirmOrder_BlockedWhenTwoAlreadyConfirmed) {
    services::OrderService order_svc(order_repo_, product_repo_);

    auto r1 = order_svc.CreateOrder({1, 100, "2099-12-31"});
    auto r2 = order_svc.CreateOrder({1, 100, "2099-12-31"});
    auto r3 = order_svc.CreateOrder({1, 100, "2099-12-31"});
    ASSERT_TRUE(r1.success);
    ASSERT_TRUE(r2.success);
    ASSERT_TRUE(r3.success);

    // 첫 번째, 두 번째 확정 -> 성공
    EXPECT_TRUE(order_svc.ConfirmOrder(r1.order_id).success);
    EXPECT_TRUE(order_svc.ConfirmOrder(r2.order_id).success);

    // 세 번째 확정 -> CONFIRMED가 이미 2개이므로 실패
    auto blocked = order_svc.ConfirmOrder(r3.order_id);
    EXPECT_FALSE(blocked.success);
    EXPECT_FALSE(blocked.error_message.empty());
}
