#pragma once
#include <istream>
#include <ostream>
#include "repositories/IOrderRepository.h"
#include "repositories/IProductRepository.h"
#include "services/OrderService.h"
#include "services/ProductionService.h"
#include "controllers/OrderController.h"
#include "views/OrderView.h"

namespace app {

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
    // 메뉴: 1=주문생성, 2=주문목록, 3=주문확정, 4=주문반려,
    //        5=주문취소, 6=생산시작, 7=출하, 0=종료
    void Run() {
        services::OrderService      order_svc(order_repo_, product_repo_);
        services::ProductionService prod_svc(order_repo_);
        controllers::OrderController ctrl(order_svc, prod_svc, input_, output_);
        views::OrderView view(output_);

        while (true) {
            output_ << u8"=== S-Semi 시료 생산주문 관리 시스템 ===\n";
            output_ << u8"1.주문생성 2.주문목록 3.주문확정 4.주문반려 5.주문취소 6.생산시작 7.출하 0.종료\n";
            output_ << "> ";

            int menu = 0;
            if (!(input_ >> menu) || menu == 0) break;

            if (menu == 2) {
                view.RenderOrderList(order_repo_.FindAll());
            } else {
                ctrl.HandleInput(menu);
            }
        }
    }

private:
    repositories::IOrderRepository&   order_repo_;
    repositories::IProductRepository& product_repo_;
    std::istream&                     input_;
    std::ostream&                     output_;
};

}  // namespace app
