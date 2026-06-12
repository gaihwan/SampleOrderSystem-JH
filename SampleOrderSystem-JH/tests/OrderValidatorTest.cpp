#include <gtest/gtest.h>
#include "utils/OrderValidator.h"

// -----------------------------------------------------------------------
// ValidateQuantity
// -----------------------------------------------------------------------

TEST(OrderValidatorTest, Valid_WhenQuantityIsPositive) {
    auto result = utils::OrderValidator::ValidateQuantity(1);
    EXPECT_TRUE(result.is_valid);
}

TEST(OrderValidatorTest, Invalid_WhenQuantityIsZero) {
    auto result = utils::OrderValidator::ValidateQuantity(0);
    EXPECT_FALSE(result.is_valid);
    EXPECT_EQ(result.error_message, "수량은 1 이상이어야 합니다");
}

TEST(OrderValidatorTest, Invalid_WhenQuantityIsNegative) {
    auto result = utils::OrderValidator::ValidateQuantity(-5);
    EXPECT_FALSE(result.is_valid);
    EXPECT_EQ(result.error_message, "수량은 1 이상이어야 합니다");
}

// -----------------------------------------------------------------------
// ValidateProductId
// -----------------------------------------------------------------------

TEST(OrderValidatorTest, Invalid_WhenProductIdNotFound) {
    std::unordered_set<int> valid_ids = {1, 2, 3};
    auto result = utils::OrderValidator::ValidateProductId(99, valid_ids);
    EXPECT_FALSE(result.is_valid);
    EXPECT_EQ(result.error_message, "시료를 찾을 수 없습니다");
}

TEST(OrderValidatorTest, Valid_WhenProductIdFound) {
    std::unordered_set<int> valid_ids = {1, 2, 3};
    auto result = utils::OrderValidator::ValidateProductId(1, valid_ids);
    EXPECT_TRUE(result.is_valid);
}

// -----------------------------------------------------------------------
// ValidateDeadline
// -----------------------------------------------------------------------

TEST(OrderValidatorTest, Invalid_WhenDeadlineFormatWrong_SlashSeparator) {
    auto result = utils::OrderValidator::ValidateDeadline("2025/01/01");
    EXPECT_FALSE(result.is_valid);
    EXPECT_EQ(result.error_message, "날짜 형식이 올바르지 않습니다 (YYYY-MM-DD)");
}

TEST(OrderValidatorTest, Invalid_WhenDeadlineFormatWrong_NoSeparator) {
    auto result = utils::OrderValidator::ValidateDeadline("20250101");
    EXPECT_FALSE(result.is_valid);
    EXPECT_EQ(result.error_message, "날짜 형식이 올바르지 않습니다 (YYYY-MM-DD)");
}

TEST(OrderValidatorTest, Valid_WhenDeadlineIsCorrectFormat) {
    auto result = utils::OrderValidator::ValidateDeadline("2025-01-01");
    EXPECT_TRUE(result.is_valid);
}