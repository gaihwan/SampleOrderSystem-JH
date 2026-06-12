#pragma once
#include "repositories/IProductRepository.h"
#include "services/OrderService.h"
#include "models/Product.h"

namespace utils {

class DummyDataGenerator {
public:
    // product_repo 에 기본 제품 3개를 저장한다 (결정론적).
    static void SeedProducts(repositories::IProductRepository& product_repo) {
        models::Product p1;
        p1.name        = "SiC-100";
        p1.batch_size  = 50;
        p1.batch_days  = 7;
        p1.yield_rate  = 0.90;
        (void)product_repo.Save(p1);

        models::Product p2;
        p2.name        = "SiC-200";
        p2.batch_size  = 25;
        p2.batch_days  = 14;
        p2.yield_rate  = 0.85;
        (void)product_repo.Save(p2);

        models::Product p3;
        p3.name        = "GaN-100";
        p3.batch_size  = 20;
        p3.batch_days  = 21;
        p3.yield_rate  = 0.80;
        (void)product_repo.Save(p3);
    }

    // order_svc 를 통해 product_id 에 대해 count 개의 주문을 생성한다.
    static void SeedOrders(services::OrderService& order_svc,
                           int product_id,
                           int count) {
        for (int i = 0; i < count; ++i) {
            services::CreateOrderRequest req;
            req.product_id = product_id;
            req.quantity   = 100;
            req.deadline   = "2099-12-31";
            (void)order_svc.CreateOrder(req);
        }
    }
};

}  // namespace utils
