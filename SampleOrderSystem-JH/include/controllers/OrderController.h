#pragma once
#include <iostream>
#include <sstream>
#include "services/OrderService.h"
#include "services/ProductionService.h"

namespace controllers {

class OrderController {
public:
    OrderController(services::OrderService&      order_service,
                    services::ProductionService& production_service,
                    std::istream&                input,
                    std::ostream&                output);

    void HandleInput();

private:
    services::OrderService&      order_service_;
    services::ProductionService& production_service_;
    std::istream&                input_;
    std::ostream&                output_;

    void HandleCreateOrder();
    void HandleConfirmOrder();
    void HandleRejectOrder();
    void HandleCancelOrder();
    void HandleStartProduction();
    void HandleRelease();
};

// --- implementations ---
inline OrderController::OrderController(services::OrderService&      order_service,
                                         services::ProductionService& production_service,
                                         std::istream&                input,
                                         std::ostream&                output)
    : order_service_(order_service)
    , production_service_(production_service)
    , input_(input)
    , output_(output) {}

inline void OrderController::HandleInput() {
    int menu;
    input_ >> menu;
    switch (menu) {
        case 1: HandleCreateOrder();     break;
        case 2: HandleConfirmOrder();    break;
        case 3: HandleRejectOrder();     break;
        case 4: HandleCancelOrder();     break;
        case 5: HandleStartProduction(); break;
        case 6: HandleRelease();         break;
        case 0: return;
        default: output_ << u8"잘못된 메뉴입니다.\n"; break;
    }
}

inline void OrderController::HandleCreateOrder() {
    int product_id;
    int quantity;
    std::string deadline;
    input_ >> product_id >> quantity >> deadline;
    auto result = order_service_.CreateOrder({product_id, quantity, deadline});
    if (result.success)
        output_ << u8"주문 생성 성공. ID: " << result.order_id << "\n";
    else
        output_ << u8"주문 생성 실패: " << result.error_message << "\n";
}

inline void OrderController::HandleConfirmOrder() {
    int order_id;
    input_ >> order_id;
    auto result = order_service_.ConfirmOrder(order_id);
    if (result.success)
        output_ << u8"주문 확정 성공.\n";
    else
        output_ << u8"주문 확정 실패: " << result.error_message << "\n";
}

inline void OrderController::HandleRejectOrder() {
    int order_id;
    input_ >> order_id;
    auto result = order_service_.RejectOrder(order_id);
    if (result.success)
        output_ << u8"주문 반려 성공.\n";
    else
        output_ << u8"주문 반려 실패: " << result.error_message << "\n";
}

inline void OrderController::HandleCancelOrder() {
    int order_id;
    input_ >> order_id;
    auto result = order_service_.CancelOrder(order_id);
    if (result.success)
        output_ << u8"주문 취소 성공.\n";
    else
        output_ << u8"주문 취소 실패: " << result.error_message << "\n";
}

inline void OrderController::HandleStartProduction() {
    int order_id;
    input_ >> order_id;
    auto result = production_service_.StartProduction(order_id);
    if (result.success)
        output_ << u8"생산 시작 성공.\n";
    else
        output_ << u8"생산 시작 실패: " << result.error_message << "\n";
}

inline void OrderController::HandleRelease() {
    int order_id;
    input_ >> order_id;
    auto result = production_service_.Release(order_id);
    if (result.success)
        output_ << u8"릴리즈 성공.\n";
    else
        output_ << u8"릴리즈 실패: " << result.error_message << "\n";
}

}  // namespace controllers
