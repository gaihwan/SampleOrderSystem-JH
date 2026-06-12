#pragma once
#include "repositories/IProductRepository.h"
#include "services/OrderService.h"

namespace utils {

// DummyDataGenerator: 테스트 및 데모용 초기 데이터를 시드한다.
// GREEN 단계에서 실제 구현 예정. 현재는 stub (아무 작업도 수행하지 않음).
class DummyDataGenerator {
public:
    // product_repo 에 기본 제품 3개를 저장한다.
    static void SeedProducts(repositories::IProductRepository& product_repo) {
        // TODO(GREEN): 3개의 기본 제품을 저장하는 구현 필요
    }

    // order_svc 를 통해 product_id 에 대해 count 개의 주문을 생성한다.
    static void SeedOrders(services::OrderService& order_svc,
                           int product_id,
                           int count) {
        // TODO(GREEN): count 개의 주문을 생성하는 구현 필요
    }
};

}  // namespace utils
