#include <gtest/gtest.h>
#include "utils/BatchCalculator.h"

// required_batches = ceil(quantity / (batch_size * yield_rate))
// estimated_yield  = required_batches * batch_size * yield_rate

TEST(BatchCalculatorTest, ReturnsCorrectBatchCount_WhenQuantityIsExact) {
    // 90 / (20 * 0.9) = 90 / 18.0 = 5.0 -> ceil(5.0) = 5
    auto result = utils::CalculateBatch(90, 20, 0.9);
    EXPECT_EQ(result.required_batches, 5);
}

TEST(BatchCalculatorTest, CeilsBatchCount_WhenQuantityHasRemainder) {
    // 100 / (20 * 0.9) = 100 / 18.0 = 5.555... -> ceil(5.556) = 6
    auto result = utils::CalculateBatch(100, 20, 0.9);
    EXPECT_EQ(result.required_batches, 6);
}

TEST(BatchCalculatorTest, ReturnsOneBatch_WhenQuantityIsOne) {
    // 1 / (20 * 0.9) = 0.0555... -> ceil(0.056) = 1
    auto result = utils::CalculateBatch(1, 20, 0.9);
    EXPECT_EQ(result.required_batches, 1);
}

TEST(BatchCalculatorTest, ReturnsCorrectEstimatedYield) {
    // required_batches = 6, estimated_yield = 6 * 20 * 0.9 = 108.0
    auto result = utils::CalculateBatch(100, 20, 0.9);
    EXPECT_DOUBLE_EQ(result.estimated_yield, 108.0);
}

TEST(BatchCalculatorTest, HandlesEdgeCase_WhenYieldRateIsOne) {
    // 20 / (20 * 1.0) = 1.0 -> ceil(1.0) = 1, estimated_yield = 1 * 20 * 1.0 = 20.0
    auto result = utils::CalculateBatch(20, 20, 1.0);
    EXPECT_EQ(result.required_batches, 1);
    EXPECT_DOUBLE_EQ(result.estimated_yield, 20.0);
}
