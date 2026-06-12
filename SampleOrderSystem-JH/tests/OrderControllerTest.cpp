#include <gtest/gtest.h>
#include <sstream>
#include "controllers/OrderController.h"
#include "repositories/InMemoryOrderRepository.h"
#include "repositories/InMemoryProductRepository.h"

class OrderControllerTest : public ::testing::Test {
protected:
    repositories::InMemoryOrderRepository   order_repo_;
    repositories::InMemoryProductRepository product_repo_;
    services::OrderService      order_service_{ order_repo_, product_repo_ };
    services::ProductionService production_service_{ order_repo_ };

    // 헬퍼: product 저장 후 id 반환
    int SaveProduct() {
        models::Product p;
        p.name       = "TestSample";
        p.batch_size = 50;
        p.batch_days = 7;
        p.yield_rate = 0.9;
        return product_repo_.Save(p);
    }

    // 헬퍼: controller 실행 후 output 반환
    std::string RunController(const std::string& input_str) {
        std::istringstream input(input_str);
        std::ostringstream output;
        controllers::OrderController ctrl(order_service_, production_service_, input, output);
        ctrl.HandleInput();
        return output.str();
    }
};

// TC-01: 메뉴 "0" 입력 시 정상 반환 (종료)
TEST_F(OrderControllerTest, ParsesMenuInput_Correctly) {
    // "0\n" 입력 -> HandleInput 정상 반환 (예외 없음)
    EXPECT_NO_THROW(RunController("0\n"));
}

// TC-02: "1" + product_id/quantity/deadline -> Order 저장됨
TEST_F(OrderControllerTest, CreateOrder_CallsServiceWithParsedArgs) {
    int pid = SaveProduct();
    std::string input = "1\n" + std::to_string(pid) + "\n100\n2099-12-31\n";
    RunController(input);
    auto all = order_repo_.FindAll();
    EXPECT_EQ(all.size(), 1u);
}

// TC-03: "2" + order_id -> status == CONFIRMED
TEST_F(OrderControllerTest, ConfirmOrder_CallsServiceWithOrderId) {
    int pid = SaveProduct();
    auto r = order_service_.CreateOrder({ pid, 100, "2099-12-31" });
    std::string input = "2\n" + std::to_string(r.order_id) + "\n";
    RunController(input);
    auto order = order_repo_.FindById(r.order_id);
    ASSERT_TRUE(order.has_value());
    EXPECT_EQ(order->status, models::OrderStatus::CONFIRMED);
}

// TC-04: "3" + order_id -> status == REJECTED
TEST_F(OrderControllerTest, RejectOrder_CallsServiceWithOrderId) {
    int pid = SaveProduct();
    auto r = order_service_.CreateOrder({ pid, 100, "2099-12-31" });
    std::string input = "3\n" + std::to_string(r.order_id) + "\n";
    RunController(input);
    auto order = order_repo_.FindById(r.order_id);
    ASSERT_TRUE(order.has_value());
    EXPECT_EQ(order->status, models::OrderStatus::REJECTED);
}

// TC-05: "4" + order_id -> status == REJECTED (cancel)
TEST_F(OrderControllerTest, CancelOrder_CallsServiceWithOrderId) {
    int pid = SaveProduct();
    auto r = order_service_.CreateOrder({ pid, 100, "2099-12-31" });
    std::string input = "4\n" + std::to_string(r.order_id) + "\n";
    RunController(input);
    auto order = order_repo_.FindById(r.order_id);
    ASSERT_TRUE(order.has_value());
    EXPECT_EQ(order->status, models::OrderStatus::REJECTED);
}

// TC-06: 잘못된 메뉴 "99" -> output에 오류 메시지 포함
TEST_F(OrderControllerTest, InvalidInput_ShowsErrorMessage) {
    std::string out = RunController("99\n");
    EXPECT_FALSE(out.empty());
    // 오류 메시지가 출력에 포함되어야 함
    EXPECT_NE(out.find(u8"잘못"), std::string::npos);  // "잘못"
}
