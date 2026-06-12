#include <gtest/gtest.h>
#include "repositories/InMemoryProductRepository.h"
#include "models/Product.h"

class InMemoryProductRepositoryTest : public ::testing::Test {
protected:
    repositories::InMemoryProductRepository repo_;

    models::Product MakeProduct(int batch_size = 100, double yield_rate = 0.9) {
        models::Product p;
        p.name       = "TestProduct";
        p.batch_size = batch_size;
        p.batch_days = 7;
        p.yield_rate = yield_rate;
        return p;
    }
};

// Save 후 반환된 id로 FindById를 호출하면 has_value=true이고 id가 일치한다.
TEST_F(InMemoryProductRepositoryTest, Save_StoresProduct) {
    int saved_id = repo_.Save(MakeProduct());

    auto result = repo_.FindById(saved_id);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->id, saved_id);
}

// Save 후 FindById(id)는 저장된 Product를 반환한다.
TEST_F(InMemoryProductRepositoryTest, FindById_ReturnsProduct_WhenExists) {
    int saved_id = repo_.Save(MakeProduct(200, 0.85));

    auto result = repo_.FindById(saved_id);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->batch_size, 200);
    EXPECT_DOUBLE_EQ(result->yield_rate, 0.85);
}

// 존재하지 않는 id로 FindById를 호출하면 nullopt를 반환한다.
TEST_F(InMemoryProductRepositoryTest, FindById_ReturnsNullopt_WhenNotFound) {
    auto result = repo_.FindById(999);

    EXPECT_FALSE(result.has_value());
}

// Product 3개를 Save한 후 FindAll은 3개를 반환한다.
TEST_F(InMemoryProductRepositoryTest, FindAll_ReturnsAllProducts) {
    repo_.Save(MakeProduct(100, 0.9));
    repo_.Save(MakeProduct(200, 0.85));
    repo_.Save(MakeProduct(150, 0.95));

    auto all = repo_.FindAll();

    EXPECT_EQ(all.size(), 3u);
}
