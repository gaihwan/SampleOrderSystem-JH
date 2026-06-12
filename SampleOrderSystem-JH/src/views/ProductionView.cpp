#include "views/ProductionView.h"
#include <iomanip>
#include <algorithm>
#include <ctime>
#include <stdexcept>

namespace views {

namespace {

// YYYY-MM-DD[T...] 형식 날짜 문자열에 days 를 더해 YYYY-MM-DD 로 반환한다.
// date_str 이 비어 있거나 파싱 실패 시 "미정" 을 반환한다.
std::string AddDays(const std::string& date_str, int days) {
    if (date_str.size() < 10) return u8"미정";
    try {
        int y = std::stoi(date_str.substr(0, 4));
        int m = std::stoi(date_str.substr(5, 2));
        int d = std::stoi(date_str.substr(8, 2));

        struct tm t = {};
        t.tm_year = y - 1900;
        t.tm_mon  = m - 1;
        t.tm_mday = d + days;
        std::mktime(&t);

        char buf[11];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d", &t);
        return std::string(buf);
    } catch (...) {
        return u8"미정";
    }
}

const char* StatusStr(models::OrderStatus s) {
    switch (s) {
        case models::OrderStatus::CONFIRMED: return "CONFIRMED";
        case models::OrderStatus::PRODUCING: return "PRODUCING";
        default:                             return "UNKNOWN";
    }
}

}  // namespace

ProductionView::ProductionView(std::ostream& output) : output_(output) {}

void ProductionView::RenderProductionStatus(
        const std::vector<models::Order>&   orders,
        const std::vector<models::Product>& products) const {

    output_ << u8"=== 생산 현황 ===\n";

    if (orders.empty()) {
        output_ << u8"  (생산 중인 주문 없음)\n";
        return;
    }

    // 헤더
    output_ << std::left
            << std::setw(8)  << u8"주문ID"
            << std::setw(12) << u8"제품명"
            << std::setw(8)  << u8"수량"
            << std::setw(12) << u8"예상수율"
            << std::setw(10) << u8"부족분"
            << std::setw(14) << u8"예상완료일"
            << u8"상태\n";
    output_ << std::string(72, '-') << "\n";

    for (const auto& o : orders) {
        // 제품 정보 조회
        std::string product_name = "-";
        int         batch_days   = 0;
        auto it = std::find_if(products.begin(), products.end(),
            [&](const models::Product& p) { return p.id == o.product_id; });
        if (it != products.end()) {
            product_name = it->name;
            batch_days   = it->batch_days;
        }

        // 부족분: 주문 수량이 예상 수율을 초과하는 양 (배치 올림으로 인해 통상 0)
        double shortage = std::max(0.0, static_cast<double>(o.quantity) - o.estimated_yield);

        // 예상완료일: 주문 생성일 + (필요배치 수 × 배치당 생산일)
        std::string completion = AddDays(o.created_at, o.required_batches * batch_days);

        output_ << std::left
                << std::setw(8)  << o.id
                << std::setw(12) << product_name
                << std::setw(8)  << o.quantity
                << std::setw(12) << o.estimated_yield
                << std::setw(10) << shortage
                << std::setw(14) << completion
                << StatusStr(o.status) << "\n";
    }
}

}  // namespace views
