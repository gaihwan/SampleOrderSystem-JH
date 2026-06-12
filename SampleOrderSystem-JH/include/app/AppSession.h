#pragma once
#include <istream>
#include <ostream>
#include "repositories/IOrderRepository.h"
#include "repositories/IProductRepository.h"

namespace app {

// AppSession: 콘솔 앱의 메인 메뉴 루프를 캡슐화한다.
// GREEN 단계에서 실제 구현 예정. 현재는 stub.
class AppSession {
public:
    AppSession(repositories::IOrderRepository&   order_repo,
               repositories::IProductRepository& product_repo,
               std::istream&                     input,
               std::ostream&                     output)
        : order_repo_(order_repo)
        , product_repo_(product_repo)
        , input_(input)
        , output_(output) {}

    // 메뉴 루프를 실행한다. 입력 스트림에서 0 을 읽으면 종료한다.
    void Run() {
        // TODO(GREEN): 메뉴 출력 및 입력 처리 루프 구현 필요
    }

private:
    repositories::IOrderRepository&   order_repo_;
    repositories::IProductRepository& product_repo_;
    std::istream&                     input_;
    std::ostream&                     output_;
};

}  // namespace app
