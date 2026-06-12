#include <gtest/gtest.h>
#include "models/Order.h"
#include "models/Product.h"

// -----------------------------------------------------------------------
// OrderModelTest
// -----------------------------------------------------------------------

TEST(OrderModelTest, DefaultStatus_IsReserved) {
    models::Order order{};
    EXPECT_EQ(order.status, models::OrderStatus::RESERVED);
}

TEST(OrderModelTest, CanAccessAllFields) {
    models::Order order{
        1,                              // id
        2,                              // product_id
        100,                            // quantity
        "2025-01-01",                   // deadline
        models::OrderStatus::CONFIRMED, // status
        5,                              // required_batches
        90.0,                           // estimated_yield
        "2025-01-01T09:00:00"          // created_at
    };

    EXPECT_EQ(order.id, 1);
    EXPECT_EQ(order.product_id, 2);
    EXPECT_EQ(order.quantity, 100);
    EXPECT_EQ(order.deadline, "2025-01-01");
    EXPECT_EQ(order.status, models::OrderStatus::CONFIRMED);
    EXPECT_EQ(order.required_batches, 5);
    EXPECT_DOUBLE_EQ(order.estimated_yield, 90.0);
    EXPECT_EQ(order.created_at, "2025-01-01T09:00:00");
}

TEST(OrderModelTest, StatusEnum_AllValuesAreCompilable) {
    // 5개 열거자가 모두 선언되어 컴파일 가능한지 검증한다.
    // 누락된 열거자는 컴파일 오류를 발생시킨다.
    [[maybe_unused]] models::OrderStatus reserved  = models::OrderStatus::RESERVED;
    [[maybe_unused]] models::OrderStatus confirmed = models::OrderStatus::CONFIRMED;
    [[maybe_unused]] models::OrderStatus producing = models::OrderStatus::PRODUCING;
    [[maybe_unused]] models::OrderStatus release   = models::OrderStatus::RELEASE;
    [[maybe_unused]] models::OrderStatus rejected  = models::OrderStatus::REJECTED;
}

// -----------------------------------------------------------------------
// ProductModelTest
// -----------------------------------------------------------------------

TEST(ProductModelTest, DefaultYieldRate_IsNinetyPercent) {
    models::Product product{};
    EXPECT_DOUBLE_EQ(product.yield_rate, 0.9);
}

TEST(ProductModelTest, CanAccessAllFields) {
    models::Product product{1, "SiC-100", 20, 3, 0.9};

    EXPECT_EQ(product.id, 1);
    EXPECT_EQ(product.name, "SiC-100");
    EXPECT_EQ(product.batch_size, 20);
    EXPECT_EQ(product.batch_days, 3);
    EXPECT_DOUBLE_EQ(product.yield_rate, 0.9);
}
