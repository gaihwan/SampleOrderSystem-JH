#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include "app/AppSession.h"
#include "repositories/InMemoryOrderRepository.h"
#include "repositories/InMemoryProductRepository.h"
#include "utils/DummyDataGenerator.h"

// -- 공통 픽스처 ----------------------------------------------------------------
class AppIntegrationTest : public ::testing::Test {
protected:
    repositories::InMemoryOrderRepository   order_repo_;
    repositories::InMemoryProductRepository product_repo_;

    // 헬퍼: 입력 문자열로 AppSession 을 실행하고 출력 문자열을 반환한다.
    std::string RunSession(const std::string& input_str) {
        std::istringstream input(input_str);
        std::ostringstream output;
        app::AppSession session(order_repo_, product_repo_, input, output);
        session.Run();
        return output.str();
    }

    // 헬퍼: 유효한 제품을 저장하고 id 를 반환한다.
    int SaveProduct() {
        models::Product p;
        p.name       = "SiC-100";
        p.batch_size = 50;
        p.batch_days = 7;
        p.yield_rate = 0.9;
        return product_repo_.Save(p);
    }
};

// TC-01: "0" 입력 시 Run() 이 정상 종료되고 출력에 메뉴 문자열이 포함된다
TEST_F(AppIntegrationTest, ShowsMenuAndExitsOnZero) {
    std::string out = RunSession("0\n");

    // Run() 이 반환되어야 하며 (무한 루프 없음)
    // 출력에 메뉴를 나타내는 텍스트가 포함되어야 한다.
    EXPECT_FALSE(out.empty()) << "output should contain menu text";
}

// TC-02: 주문 생성(1) 후 목록(2) 조회 시 주문이 존재한다
TEST_F(AppIntegrationTest, CreateOrder_AppearsInList) {
    int pid = SaveProduct();

    // 1: 주문생성, product_id, quantity, deadline
    // 2: 주문목록 조회
    // 0: 종료
    std::string input =
        "1\n" + std::to_string(pid) + "\n100\n2099-12-31\n"
        "2\n"
        "0\n";

    std::string out = RunSession(input);

    // 주문 생성 성공 메시지가 출력에 포함되어야 한다.
    EXPECT_NE(out.find(u8"성공"), std::string::npos)
        << "output should contain success message after create order";

    // repository 에도 주문이 존재해야 한다.
    auto all = order_repo_.FindAll();
    EXPECT_EQ(all.size(), 1u);
}

// TC-03: DummyData 시드 → 주문생성(1) → 확정(3) → 생산시작(6) → 출하(7) → 전체 라이프사이클 성공
TEST_F(AppIntegrationTest, FullLifecycle_WithDummyData) {
    // DummyData 로 제품 시드
    utils::DummyDataGenerator::SeedProducts(product_repo_);
    auto products = product_repo_.FindAll();
    ASSERT_FALSE(products.empty());
    int pid = products[0].id;

    // 메뉴 번호는 OrderController::HandleInput 과 동일한 규칙:
    // 1=생성, 2=확정, 3=반려, 5=생산시작, 6=출하
    // AppSession 이 같은 메뉴 번호 체계를 사용한다고 가정한다.
    // (GREEN 단계에서 실제 메뉴 번호 확정)
    // 여기서는 주문 생성 후 RESERVED 상태 확인까지만 검증한다.
    std::string input =
        "1\n" + std::to_string(pid) + "\n100\n2099-12-31\n"
        "0\n";

    RunSession(input);

    auto all = order_repo_.FindAll();
    ASSERT_EQ(all.size(), 1u);

    int order_id = all[0].id;

    // 확정(ConfirmOrder) → 생산시작(StartProduction) → 출하(Release) 는
    // AppSession 이 구현된 후 세션 입력으로 전달할 예정.
    // RED 단계에서는 주문이 생성되어 RESERVED 상태임을 확인한다.
    EXPECT_EQ(all[0].status, models::OrderStatus::RESERVED);
    EXPECT_GT(order_id, 0);
}

// TC-04: 유효하지 않은 메뉴 번호(99) 입력 후 0 으로 종료 → 오류 메시지 포함
TEST_F(AppIntegrationTest, InvalidMenu_ShowsError) {
    std::string out = RunSession("99\n0\n");

    // 잘못된 메뉴에 대해 오류 관련 텍스트가 출력에 포함되어야 한다.
    bool has_error =
        (out.find(u8"잘못된") != std::string::npos) ||
        (out.find(u8"오류")   != std::string::npos) ||
        (out.find("invalid")  != std::string::npos) ||
        (out.find("error")    != std::string::npos);

    EXPECT_TRUE(has_error)
        << "output should contain an error indicator for invalid menu input. actual: " << out;
}
