#include <gtest/gtest.h>
#include "repositories/InMemoryOrderRepository.h"
#include "models/Order.h"

class InMemoryOrderRepositoryTest : public ::testing::Test {
protected:
    repositories::InMemoryOrderRepository repo_;

    // 헬퍼: 기본 Order 생성
    models::Order MakeOrder(int product_id = 1, int quantity = 100,
                            models::OrderStatus status = models::OrderStatus::RESERVED) {
        models::Order o;
        o.product_id = product_id;
        o.quantity   = quantity;
        o.deadline   = "2025-12-31";
        o.status     = status;
        return o;
    }
};

// Save 호출 시 auto-increment id(1, 2, ...)가 순서대로 부여된다.
TEST_F(InMemoryOrderRepositoryTest, Save_AssignsAutoIncrementId) {
    int id1 = repo_.Save(MakeOrder());
    int id2 = repo_.Save(MakeOrder());

    EXPECT_EQ(id1, 1);
    EXPECT_EQ(id2, 2);
}

// Save 후 반환된 id로 FindById를 호출하면 해당 Order를 찾는다.
TEST_F(InMemoryOrderRepositoryTest, FindById_ReturnsOrder_WhenExists) {
    int saved_id = repo_.Save(MakeOrder());

    auto result = repo_.FindById(saved_id);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->id, saved_id);
}

// 존재하지 않는 id로 FindById를 호출하면 nullopt를 반환한다.
TEST_F(InMemoryOrderRepositoryTest, FindById_ReturnsNullopt_WhenNotFound) {
    auto result = repo_.FindById(999);

    EXPECT_FALSE(result.has_value());
}

// 3개의 Order를 Save한 후 FindAll은 3개를 반환한다.
TEST_F(InMemoryOrderRepositoryTest, FindAll_ReturnsAllOrders) {
    repo_.Save(MakeOrder());
    repo_.Save(MakeOrder());
    repo_.Save(MakeOrder());

    auto all = repo_.FindAll();

    EXPECT_EQ(all.size(), 3u);
}

// RESERVED 2개와 CONFIRMED 1개를 Save한 후 FindByStatus(RESERVED)는 2개를 반환한다.
TEST_F(InMemoryOrderRepositoryTest, FindByStatus_FiltersCorrectly) {
    repo_.Save(MakeOrder(1, 100, models::OrderStatus::RESERVED));
    repo_.Save(MakeOrder(1, 100, models::OrderStatus::RESERVED));
    repo_.Save(MakeOrder(1, 100, models::OrderStatus::CONFIRMED));

    auto reserved = repo_.FindByStatus(models::OrderStatus::RESERVED);

    EXPECT_EQ(reserved.size(), 2u);
}

// Save 후 status를 CONFIRMED으로 변경하여 Update하면 FindById 결과에 반영된다.
TEST_F(InMemoryOrderRepositoryTest, Update_ChangesOrderStatus) {
    int saved_id = repo_.Save(MakeOrder());

    models::Order updated = MakeOrder();
    updated.id     = saved_id;
    updated.status = models::OrderStatus::CONFIRMED;
    repo_.Update(updated);

    auto result = repo_.FindById(saved_id);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->status, models::OrderStatus::CONFIRMED);
}

// product_id=1 × 2개, product_id=2 × 1개 Save 후 FindByProductId(1)은 2개를 반환한다.
TEST_F(InMemoryOrderRepositoryTest, FindByProductId_ReturnsMatchingOrders) {
    repo_.Save(MakeOrder(1, 100));
    repo_.Save(MakeOrder(1, 200));
    repo_.Save(MakeOrder(2, 150));

    auto orders = repo_.FindByProductId(1);

    EXPECT_EQ(orders.size(), 2u);
}
