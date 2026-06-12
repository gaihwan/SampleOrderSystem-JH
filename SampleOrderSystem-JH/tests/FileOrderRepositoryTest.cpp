#include <gtest/gtest.h>
#include <cstdio>   // std::remove
#include "repositories/FileOrderRepository.h"

class FileOrderRepositoryTest : public ::testing::Test {
protected:
    const std::string test_file_ = "test_orders_temp.json";

    void TearDown() override {
        std::remove(test_file_.c_str());
    }

    models::Order MakeOrder(int product_id = 1, int quantity = 100,
                            models::OrderStatus status = models::OrderStatus::RESERVED) {
        models::Order o;
        o.product_id = product_id;
        o.quantity   = quantity;
        o.deadline   = "2099-12-31";
        o.status     = status;
        o.required_batches  = 3;
        o.estimated_yield   = 135.0;
        return o;
    }
};

// TC-01: Save 후 FindAll size == 1
TEST_F(FileOrderRepositoryTest, Save_WritesJsonFile) {
    repositories::FileOrderRepository repo(test_file_);
    repo.Save(MakeOrder());

    auto all = repo.FindAll();
    EXPECT_EQ(all.size(), 1u);
}

// TC-02: 저장 후 FindAll로 내용 검증 (quantity, status)
TEST_F(FileOrderRepositoryTest, FindAll_ReadsFromJsonFile) {
    repositories::FileOrderRepository repo(test_file_);
    int id = repo.Save(MakeOrder(2, 200, models::OrderStatus::CONFIRMED));

    auto all = repo.FindAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].id,         id);
    EXPECT_EQ(all[0].product_id, 2);
    EXPECT_EQ(all[0].quantity,   200);
    EXPECT_EQ(all[0].status,     models::OrderStatus::CONFIRMED);
}

// TC-03: 재로드 후 데이터 동일 (새 인스턴스 생성)
TEST_F(FileOrderRepositoryTest, PersistsAcrossReload) {
    int saved_id = 0;
    {
        repositories::FileOrderRepository repo(test_file_);
        saved_id = repo.Save(MakeOrder(1, 100));
    }
    // 새 인스턴스로 재로드
    repositories::FileOrderRepository repo2(test_file_);
    auto all = repo2.FindAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].id,       saved_id);
    EXPECT_EQ(all[0].quantity, 100);
}

// TC-04: Update 후 FindById 결과 반영
TEST_F(FileOrderRepositoryTest, Update_OverwritesExistingEntry) {
    repositories::FileOrderRepository repo(test_file_);
    int id = repo.Save(MakeOrder());

    models::Order updated = MakeOrder();
    updated.id     = id;
    updated.status = models::OrderStatus::CONFIRMED;
    ASSERT_TRUE(repo.Update(updated));

    auto result = repo.FindById(id);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->status, models::OrderStatus::CONFIRMED);
}

// TC-05: 파일 미존재 시 빈 벡터 반환 (예외 없음)
TEST_F(FileOrderRepositoryTest, HandlesEmptyFile_GracefullyReturnsEmpty) {
    repositories::FileOrderRepository repo("nonexistent_file_xyz.json");
    auto all = repo.FindAll();
    EXPECT_EQ(all.size(), 0u);
}
