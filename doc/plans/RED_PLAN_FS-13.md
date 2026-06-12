# RED_PLAN_FS-13 — Release 빌드 수정 + CLI 앱 + DummyDataGenerator

> 작성일: 2026-06-12
> 브랜치: feat/fs-13-release-app
> 연계: [PLAN.md §3 Phase 7 — FS-13](../../PLAN.md)

## 1. Slice 개요

| 항목 | 내용 |
|------|------|
| **Slice ID** | FS-13 |
| **레이어** | Build / Utils / Integration |
| **핵심 책임** | Release 빌드 수정, CLI 앱 진입점, DummyDataGenerator |
| **의존 Slice** | FS-01~FS-12 (모두) |

### 배경 / 문제 정의

현재 프로젝트는 아래 세 가지 문제로 Release Mode 동작이 불가하다.

| # | 문제 | 원인 |
|---|------|------|
| P-01 | `C1041` PDB 충돌 | vcxproj Release 설정에 `/FS` 플래그 누락 |
| P-02 | 테스트 파일이 Release에 포함 | `tests/*.cpp`가 모든 구성에 컴파일됨 → GTest 없이 빌드 실패 |
| P-03 | CLI 앱 진입점 없음 | `main.cpp` = GTest 런처만 있고, 실제 메뉴 루프 없음 |

---

## 2. 서브 슬라이스 구조

FS-13은 세 개의 논리적 작업 단위로 구성된다.

### FS-13-A: Build Architecture Fix (chore)

**목표**: Debug=Test 실행, Release=CLI 앱 실행으로 구성 분리

| 항목 | 변경 내용 |
|------|----------|
| vcxproj `/FS` | Release|x64, Debug|x64 모두 `<AdditionalOptions>/FS</AdditionalOptions>` 추가 |
| test 파일 Release 제외 | `tests/*.cpp` 각 파일에 `Release|x64` 설정에서 `<ExcludedFromBuild>true</ExcludedFromBuild>` |
| main.cpp Release 제외 | 현재 GTest launcher main.cpp를 Release|x64 에서 제외 |
| app_main.cpp 추가 | Debug|x64에서 제외, Release|x64에서 포함 |

**검증 기준**:
- Debug|x64 빌드 성공 + 72개 테스트 통과
- Release|x64 빌드 성공 (에러 없음)

---

### FS-13-B: DummyDataGenerator (TDD)

**목표**: 제품/주문 샘플 데이터 생성 유틸리티

| 항목 | 내용 |
|------|------|
| 파일 | `include/utils/DummyDataGenerator.h` |
| 책임 | InMemoryProductRepository에 샘플 제품 3종 Seed |
| 제품 데이터 | SiC-100 (batch_size=50, batch_days=7, yield=0.90), SiC-200 (batch_size=25, batch_days=14, yield=0.85), GaN-100 (batch_size=20, batch_days=21, yield=0.80) |

**테스트 케이스 목록**:

| # | 테스트 이름 | 검증 시나리오 |
|---|------------|-------------|
| TC-01 | `SeedProducts_InsertsThreeProducts` | SeedProducts 호출 후 FindAll() 결과가 3개 |
| TC-02 | `SeedProducts_ProductsHaveValidFields` | 각 제품의 name, batch_size, yield_rate가 0보다 큰 유효값 |
| TC-03 | `SeedProducts_IsDeterministic` | 두 번 호출해도 항상 동일한 제품 목록 생성 (idempotent) |
| TC-04 | `SeedOrders_InsertsExpectedOrders` | SeedOrders 호출 후 주문이 생성됨 |
| TC-05 | `SeedOrders_OrdersAreInReservedStatus` | 생성된 주문의 status = RESERVED |

---

### FS-13-C: CLI App Main + 통합 시나리오 (TDD)

**목표**: 실제 동작 가능한 CLI 앱 진입점 + DummyData 기반 통합 시나리오 테스트

| 항목 | 내용 |
|------|------|
| 파일 | `src/app_main.cpp` (Release 전용) |
| 초기화 | InMemoryProductRepository + DummyDataGenerator::SeedProducts |
| 저장소 | FileOrderRepository (orders.jsonl) |
| 컨트롤러 | OrderController (std::cin / std::cout) |
| 뷰 | OrderView, ProductionView |
| 메뉴 | 1=주문생성, 2=주문목록, 3=주문확정, 4=주문거절, 5=주문취소, 6=생산시작, 7=출하, 0=종료 |

**통합 시나리오 테스트 케이스** (`tests/AppIntegrationTest.cpp`):

| # | 테스트 이름 | 검증 시나리오 |
|---|------------|-------------|
| TC-01 | `AppSession_ShowsMenuAndExitsOnZero` | "0\n" 입력 시 앱이 정상 종료 (출력에 메뉴 포함) |
| TC-02 | `AppSession_CreateOrder_AppearsInList` | 주문 생성(1) → 목록 조회(2) → 목록에 주문 ID 포함 |
| TC-03 | `AppSession_FullLifecycle_WithDummyData` | DummyData 시드 → 주문생성 → 확정 → 생산시작 → 출하 전 과정 성공 |
| TC-04 | `AppSession_InvalidMenu_ShowsError` | 유효하지 않은 메뉴 입력 시 오류 메시지 출력 |

---

## 3. 설계 결정

### app_main.cpp 구조

```cpp
// src/app_main.cpp  — Release 전용 진입점
#include "repositories/InMemoryProductRepository.h"
#include "repositories/FileOrderRepository.h"
#include "services/OrderService.h"
#include "services/ProductionService.h"
#include "controllers/OrderController.h"
#include "views/OrderView.h"
#include "utils/DummyDataGenerator.h"

int main() {
    repositories::InMemoryProductRepository product_repo;
    utils::DummyDataGenerator::SeedProducts(product_repo);

    repositories::FileOrderRepository order_repo("orders.jsonl");
    services::OrderService order_svc(order_repo, product_repo);
    services::ProductionService prod_svc(order_repo);
    controllers::OrderController ctrl(order_svc, prod_svc, std::cin, std::cout);
    views::OrderView order_view(std::cout);

    bool running = true;
    while (running) {
        // 메뉴 출력 → 입력 → HandleInput
        std::cout << "\n=== S-Semi 시료 생산주문 관리 시스템 ===\n"
                  << "1. 주문 생성   2. 주문 목록\n"
                  << "3. 주문 확정   4. 주문 거절\n"
                  << "5. 주문 취소   6. 생산 시작\n"
                  << "7. 출하        0. 종료\n"
                  << "> ";
        int menu = 0;
        if (!(std::cin >> menu) || menu == 0) break;
        if (menu == 2) {
            order_view.RenderOrderList(order_repo.FindAll());
        } else {
            ctrl.HandleInput(); // 내부에서 menu를 재읽음 → 리팩 필요 시 조정
        }
    }
    return 0;
}
```

> **설계 주의**: `OrderController::HandleInput()`이 내부에서 menu 값을 읽으므로, app_main에서 menu를 미리 읽으면 충돌. → `HandleInput()` 시그니처를 `HandleInput(int menu)` 로 수정하거나, app_main에서 전체 입력을 위임하는 방식으로 조정 필요 → GREEN 단계에서 확정

### DummyDataGenerator 구조

```cpp
// include/utils/DummyDataGenerator.h
namespace utils {
class DummyDataGenerator {
public:
    static void SeedProducts(repositories::IProductRepository& repo);
    static void SeedOrders(services::OrderService& svc, int product_id, int count = 3);
};
}
```

### vcxproj 변경 전략

- `/FS` 추가: `ItemDefinitionGroup` → `ClCompile` → `AdditionalOptions`
- 테스트 파일 Release 제외: 각 `<ClCompile Include="tests\*.cpp">` 에 `<ExcludedFromBuild Condition="'$(Configuration)|$(Platform)'=='Release|x64'">true</ExcludedFromBuild>` 추가
- main.cpp 도 Release|x64에서 제외
- app_main.cpp: Debug|x64에서 제외, Release|x64에서 포함

---

## 4. 작업 순서 (TDD 사이클)

```
[FS-13-A] vcxproj 수정 (chore) 
    → Debug 빌드 + 테스트 통과 확인
    → Release 빌드 에러 감소 확인

[FS-13-B] DummyDataGenerator TDD
    RED: DummyDataGeneratorTest.cpp 작성 (5개 TC 실패)
    GREEN: DummyDataGenerator.h 구현
    커밋①

[FS-13-C] CLI App + AppIntegration TDD
    RED: AppIntegrationTest.cpp 작성 (4개 TC 실패)
    GREEN: app_main.cpp + OrderController 시그니처 조정
    커밋②

[REVIEW → REFACTOR → 빌드 검증 → 커밋③]
    Release|x64 빌드 성공 + CLI 앱 실행 확인
```

---

## 5. 완료 기준

- [ ] `Debug|x64` 빌드 성공, 기존 72개 + 신규 9개 = **81개 이상** 테스트 그린
- [ ] `Release|x64` 빌드 성공 (에러/경고 없음)
- [ ] `Release` 실행파일로 CLI 앱이 정상 동작 (메뉴 루프, 주문 생성, 목록 조회)
- [ ] DummyData 3종 제품이 앱 시작 시 자동 로드됨
