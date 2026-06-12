#include <iostream>
#include "repositories/InMemoryProductRepository.h"
#include "repositories/FileOrderRepository.h"
#include "app/AppSession.h"
#include "utils/DummyDataGenerator.h"

int main() {
    repositories::InMemoryProductRepository product_repo;
    utils::DummyDataGenerator::SeedProducts(product_repo);

    repositories::FileOrderRepository order_repo("orders.jsonl");

    app::AppSession session(order_repo, product_repo, std::cin, std::cout);
    session.Run();

    return 0;
}
