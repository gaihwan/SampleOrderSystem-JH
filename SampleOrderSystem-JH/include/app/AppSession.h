#pragma once
#include <istream>
#include <ostream>
#include <iomanip>
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
            output_ << u8"\n=== S-Semi 시료 생산주문 관리 시스템 ===\n";
            output_ << u8"  1.주문생성  2.주문목록  3.주문확정  4.주문반려\n";
            output_ << u8"  5.주문취소  6.생산시작  7.출하       0.종료\n";
            output_ << u8"> ";

            int menu = 0;
            if (!(input_ >> menu) || menu == 0) break;

            if (menu == 1) {
                // 주문 생성: 사용 가능한 제품 목록을 먼저 보여준다
                RenderProductList();
                ctrl.HandleInput(menu);
            } else if (menu == 2) {
                // 주문 목록 직접 출력
                view.RenderOrderList(order_repo_.FindAll());
            } else if (menu >= 3 && menu <= 7) {
                // 주문 ID 입력이 필요한 메뉴: 현재 주문 목록을 먼저 보여준다
                output_ << u8"[현재 주문 목록]\n";
                view.RenderOrderList(order_repo_.FindAll());
                ctrl.HandleInput(menu);
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

    // 등록된 제품 목록을 표 형태로 출력한다.
    void RenderProductList() const {
        auto products = product_repo_.FindAll();
        output_ << u8"[사용 가능한 제품 목록]\n";
        output_ << std::left
                << std::setw(6)  << u8"ID"
                << std::setw(12) << u8"제품명"
                << std::setw(10) << u8"배치크기"
                << std::setw(10) << u8"납기(일)"
                << u8"수율\n";
        for (const auto& p : products) {
            output_ << std::left
                    << std::setw(6)  << p.id
                    << std::setw(12) << p.name
                    << std::setw(10) << p.batch_size
                    << std::setw(10) << p.batch_days
                    << p.yield_rate << "\n";
        }
    }
};

}  // namespace app
