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

// TC-03: DummyData 시드 → AppSession 경유 전체 라이프사이클 (생성→확정→생산시작→출하)
TEST_F(AppIntegrationTest, FullLifecycle_WithDummyData) {
    // DummyData로 제품 시드
    utils::DummyDataGenerator::SeedProducts(product_repo_);
    auto products = product_repo_.FindAll();
    ASSERT_FALSE(products.empty());
    int pid = products[0].id;

    // AppSession 메뉴 체계:
    // 1=주문생성(pid, qty, deadline), 3=주문확정(order_id),
    // 6=생산시작(order_id), 7=출하(order_id), 0=종료
    std::string input =
        "1\n" + std::to_string(pid) + "\n100\n2099-12-31\n"   // 주문 생성
        "3\n1\n"                                               // 주문 확정 (id=1)
        "6\n1\n"                                               // 생산 시작
        "7\n1\n"                                               // 출하
        "0\n";                                                  // 종료

    std::string out = RunSession(input);

    // 출하까지 성공 메시지 포함
    EXPECT_NE(out.find(u8"성공"), std::string::npos)
        << "output should contain success messages. actual:\n" << out;

    // 최종 상태: RELEASE
    auto all = order_repo_.FindAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].status, models::OrderStatus::RELEASE)
        << "order should reach RELEASE status through AppSession";
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
