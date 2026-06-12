# RED_PLAN_FS-06 — OrderService 주문 생성

> 작성일: 2026-06-12  
> 브랜치: feat/fs-06-order-service-create  
> 연계: [PLAN.md §3 Phase 3 — FS-06](../../PLAN.md)

---

## 1. Slice 개요

| 항목 | 내용 |
|------|------|
| **Slice ID** | FS-06 |
| **레이어** | Service |
| **핵심 책임** | 주문 생성 유효성 검사, 배치 자동 계산, Order 저장 |
| **의존 Slice** | FS-01 (BatchCalculator), FS-02 (OrderValidator), FS-04 (IOrderRepository), FS-05 (IProductRepository) |

---

## 2. PLAN.md 연계

```
| FS-06 | OrderService | Service | CreateOrder 유효성 검사·배치 계산·저장 | FS-01, FS-02, FS-04, FS-05 |
```

---

## 3. 설계 결정

### 인터페이스 설계

```cpp
// include/services/OrderService.h
#pragma once
#include <optional>
#include <string>
#include "models/Order.h"
#include "repositories/IOrderRepository.h"
#include "repositories/IProductRepository.h"

namespace services {

struct CreateOrderRequest {
    int         product_id = 0;
    int         quantity   = 0;
    std::string deadline;
};

struct ServiceResult {
    bool        success = false;
    std::string error_message;
    int         order_id = 0;   // 성공 시에만 유효
};

class OrderService {
public:
    OrderService(repositories::IOrderRepository&  order_repo,
                 repositories::IProductRepository& product_repo);

    [[nodiscard]] ServiceResult CreateOrder(const CreateOrderRequest& req);

private:
    repositories::IOrderRepository&  order_repo_;
    repositories::IProductRepository& product_repo_;
};

}  // namespace services
```

### 설계 결정 사항

| 결정 | 이유 |
|------|------|
| Repository를 참조로 주입 | 소유권 없음, 수명은 호출자가 관리 — InMemory/Mock 교체 가능 |
| `CreateOrderRequest` 별도 구조체 | 파라미터 수 증가 시 API 변경 최소화 |
| `ServiceResult` 반환 | 예외 없이 성공/실패·오류 메시지를 타입 안전하게 전달 |
| `BatchCalculator` 직접 호출 (정적 함수) | 상태 없음, 의존성 주입 불필요 |
| `OrderValidator` 직접 호출 (정적 함수) | 상태 없음, 의존성 주입 불필요 |

### CreateOrder 처리 흐름

```
1. ValidateQuantity(req.quantity)           → 실패 시 ServiceResult{false, error}
2. ValidateDeadline(req.deadline)           → 실패 시 ServiceResult{false, error}
3. product_repo_.FindById(req.product_id)  → 없으면 ServiceResult{false, "시료를 찾을 수 없습니다"}
4. CalculateBatch(quantity, batch_size, yield_rate) → BatchResult
5. Order 구성 (status=RESERVED, required_batches, estimated_yield 세팅)
6. order_repo_.Save(order)                 → ServiceResult{true, "", saved_id}
```

### 의존성 처리

- `IOrderRepository`, `IProductRepository`는 **실제 InMemory 구현체**를 사용 (Mock 불필요)
- 테스트 픽스처에서 repository 인스턴스를 직접 생성하여 `OrderService` 생성자에 주입

---

## 4. 테스트 케이스 목록

| # | 테스트 이름 | 검증 동작 | 입력 | 기대 출력 |
|---|------------|---------|------|---------|
| 1 | `CreateOrder_ReturnsOrderWithReservedStatus` | 생성된 주문의 초기 상태 | 유효한 요청 (product 등록 후) | `FindById(order_id).status == RESERVED` |
| 2 | `CreateOrder_CalculatesRequiredBatchesAutomatically` | 배치 수 자동 계산 | quantity=100, batch_size=50, yield_rate=0.9 | `required_batches == ceil(100 / (50*0.9)) == 3` |
| 3 | `CreateOrder_CalculatesEstimatedYieldAutomatically` | 예상 생산량 자동 계산 | quantity=100, batch_size=50, yield_rate=0.9 | `estimated_yield == 3 * 50 * 0.9 == 135.0` |
| 4 | `CreateOrder_Fails_WhenProductNotFound` | 존재하지 않는 product_id | `product_id=999` (등록 안 됨) | `success == false` |
| 5 | `CreateOrder_Fails_WhenQuantityIsZero` | 수량 0 유효성 검사 | `quantity=0` | `success == false` |
| 6 | `CreateOrder_Fails_WhenDeadlineFormatInvalid` | 날짜 형식 오류 | `deadline="20251231"` (YYYY-MM-DD 아님) | `success == false` |

---

## 5. stub 전략

`OrderService::CreateOrder`를 빈 구현으로 작성한다:

```cpp
// src/services/OrderService.cpp (stub)
#include "services/OrderService.h"

namespace services {

OrderService::OrderService(repositories::IOrderRepository&  order_repo,
                           repositories::IProductRepository& product_repo)
    : order_repo_(order_repo), product_repo_(product_repo) {}

ServiceResult OrderService::CreateOrder(const CreateOrderRequest& req) {
    return ServiceResult{};  // stub: success=false, order_id=0
}

}  // namespace services
```

이 stub 상태에서:
- `CreateOrder_Fails_*` 3개 → `success==false` 이므로 **우연히 통과**
- `CreateOrder_Returns*` 및 `CreateOrder_Calculates*` 3개 → 실패 (RED 확인 대상)

RED 단계 목표: 6개 테스트가 컴파일 성공 후 최소 3개 이상 실패함을 확인한다.

---

## 6. 파일 목록

| 역할 | 경로 |
|------|------|
| Service 헤더 (stub) | `SampleOrderSystem-JH/include/services/OrderService.h` |
| Service 구현 (stub) | `SampleOrderSystem-JH/src/services/OrderService.cpp` |
| 테스트 | `SampleOrderSystem-JH/tests/OrderServiceCreateTest.cpp` |

---

## 7. 완료 기준

- [ ] RED: 6개 테스트 컴파일 성공 후 3개 이상 실패
- [ ] GREEN: 전체 테스트 PASSED (기존 29개 + 신규 6개 = 35개)
- [ ] 커밋①: GREEN 상태 feat 커밋
- [ ] REVIEW: 코드 리뷰 + 사용자 승인
- [ ] 커밋②: REFACTOR 반영 후 최종 커밋
