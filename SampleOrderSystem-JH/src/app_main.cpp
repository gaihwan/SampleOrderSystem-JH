#include <iostream>
#include <windows.h>
#include "repositories/InMemoryProductRepository.h"
#include "repositories/FileOrderRepository.h"
#include "app/AppSession.h"
#include "utils/DummyDataGenerator.h"

int main() {
    // Windows 콘솔을 UTF-8 코드 페이지로 설정하여 한글 출력 정상화
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    repositories::InMemoryProductRepository product_repo;
    utils::DummyDataGenerator::SeedProducts(product_repo);

    repositories::FileOrderRepository order_repo("orders.jsonl");

    app::AppSession session(order_repo, product_repo, std::cin, std::cout);
    session.Run();

    return 0;
}
