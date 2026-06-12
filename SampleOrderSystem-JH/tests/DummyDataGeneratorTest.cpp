#include <gtest/gtest.h>
#include "utils/DummyDataGenerator.h"
#include "repositories/InMemoryProductRepository.h"
#include "repositories/InMemoryOrderRepository.h"
#include "services/OrderService.h"

// -- 공통 픽스처 ----------------------------------------------------------------
class DummyDataGeneratorTest : public ::testing::Test {
protected:
    repositories::InMemoryProductRepository product_repo_;
    repositories::InMemoryOrderRepository   order_repo_;
};

// TC-01: SeedProducts 호출 후 제품이 3개 저장된다
TEST_F(DummyDataGeneratorTest, SeedProducts_InsertsThreeProducts) {
    utils::DummyDataGenerator::SeedProducts(product_repo_);

    auto all = product_repo_.FindAll();

    EXPECT_EQ(all.size(), 3u);
}

// TC-02: 시드된 제품들은 유효한 필드를 가진다 (이름 비어있지 않음, batch_size > 0, yield_rate > 0.0)
TEST_F(DummyDataGeneratorTest, SeedProducts_ProductsHaveValidFields) {
    utils::DummyDataGenerator::SeedProducts(product_repo_);

    auto all = product_repo_.FindAll();
    ASSERT_EQ(all.size(), 3u);

    for (const auto& p : all) {
        EXPECT_FALSE(p.name.empty())        << "name should not be empty";
        EXPECT_GT(p.batch_size, 0)          << "batch_size should be positive";
        EXPECT_GT(p.yield_rate, 0.0)        << "yield_rate should be positive";
    }
}

// TC-03: SeedProducts 를 두 번 호출해도 첫 번째 제품 이름이 동일하다 (결정론적)
TEST_F(DummyDataGeneratorTest, SeedProducts_IsDeterministic) {
    repositories::InMemoryProductRepository repo_a;
    repositories::InMemoryProductRepository repo_b;

    utils::DummyDataGenerator::SeedProducts(repo_a);
    utils::DummyDataGenerator::SeedProducts(repo_b);

    auto all_a = repo_a.FindAll();
    auto all_b = repo_b.FindAll();

    ASSERT_FALSE(all_a.empty());
    ASSERT_FALSE(all_b.empty());

    // 동일한 시드이므로 저장 순서 기준 첫 번째 제품의 이름이 같아야 한다.
    // FindAll 의 순서가 일치하지 않을 수 있으므로 id=1 을 기준으로 비교한다.
    auto first_a = repo_a.FindById(1);
    auto first_b = repo_b.FindById(1);
    ASSERT_TRUE(first_a.has_value());
    ASSERT_TRUE(first_b.has_value());
    EXPECT_EQ(first_a->name, first_b->name);
}

// TC-04: SeedOrders(svc, product_id, 3) 후 주문이 3개 저장된다
TEST_F(DummyDataGeneratorTest, SeedOrders_InsertsExpectedOrders) {
    // 제품을 먼저 시드하고 첫 번째 id 를 사용한다.
    utils::DummyDataGenerator::SeedProducts(product_repo_);
    auto products = product_repo_.FindAll();
    ASSERT_FALSE(products.empty());
    int product_id = products[0].id;

    services::OrderService order_svc(order_repo_, product_repo_);
    utils::DummyDataGenerator::SeedOrders(order_svc, product_id, 3);

    auto all = order_repo_.FindAll();

    EXPECT_EQ(all.size(), 3u);
}

// TC-05: SeedOrders 로 생성된 주문들은 RESERVED 상태이다
TEST_F(DummyDataGeneratorTest, SeedOrders_OrdersAreInReservedStatus) {
    utils::DummyDataGenerator::SeedProducts(product_repo_);
    auto products = product_repo_.FindAll();
    ASSERT_FALSE(products.empty());
    int product_id = products[0].id;

    services::OrderService order_svc(order_repo_, product_repo_);
    utils::DummyDataGenerator::SeedOrders(order_svc, product_id, 3);

    auto all = order_repo_.FindAll();
    ASSERT_EQ(all.size(), 3u);

    for (const auto& o : all) {
        EXPECT_EQ(o.status, models::OrderStatus::RESERVED)
            << "order id=" << o.id << " should be RESERVED";
    }
}
