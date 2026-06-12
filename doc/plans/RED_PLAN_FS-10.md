# RED_PLAN_FS-10 — OrderController 콘솔 입력 파싱 및 Service 연결

> 작성일: 2026-06-12  
> 브랜치: feat/fs-10-order-controller  
> 연계: [PLAN.md §3 Phase 5 — FS-10](../../PLAN.md)

---

## 1. Slice 개요

| 항목 | 내용 |
|------|------|
| **Slice ID** | FS-10 |
| **레이어** | Controller |
| **핵심 책임** | 콘솔 메뉴 입력 파싱, Service 호출, 결과 출력 위임 |
| **의존 Slice** | FS-06 (OrderService), FS-07 (상태 전이), FS-08 (ProductionService) |

---

## 2. PLAN.md 연계

> PLAN.md Feature Slice 목록 중 해당 항목:

```
| FS-10 | OrderController | Controller | 콘솔 입력 파싱, 메뉴 흐름 | FS-07, FS-08 |
```

---

## 3. 설계 결정

### 핵심 설계 원칙 — 콘솔 I/O 의존성 주입으로 테스트 가능하게

콘솔 입출력을 `std::istream`/`std::ostream` 참조로 주입하여 테스트에서
`std::istringstream`/`std::ostringstream`으로 대체 가능하게 설계한다.

### 인터페이스 설계

```cpp
// include/controllers/OrderController.h
#pragma once
#include "services/OrderService.h"
#include "services/ProductionService.h"
#include <istream>
#include <ostream>

namespace controllers {

class OrderController {
public:
    OrderController(services::OrderService&      order_service,
                    services::ProductionService& production_service,
                    std::istream&                input,
                    std::ostream&                output);

    // 단일 메뉴 사이클: 입력 읽기 → 파싱 → Service 호출 → 출력
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

}  // namespace controllers
```

### 메뉴 번호 → 동작 매핑 (콘솔 입력 포맷)

```
1 → 주문 생성  (product_id, quantity, deadline 추가 입력)
2 → 주문 확정  (order_id 추가 입력)
3 → 주문 반려  (order_id 추가 입력)
4 → 주문 취소  (order_id 추가 입력)
5 → 생산 시작  (order_id 추가 입력)
6 → 릴리즈     (order_id 추가 입력)
0 → 종료       (HandleInput에서 처리 없이 반환)
기타 → "잘못된 메뉴입니다" 출력
```

### 설계 결정 사항

| 결정 | 이유 |
|------|------|
| `std::istream`/`std::ostream` 참조 주입 | 테스트에서 `istringstream`/`ostringstream`으로 대체 가능 |
| `HandleInput()` 단일 사이클 | 루프 제어는 상위(main)에 위임, 테스트를 단순하게 유지 |
| 성공/실패 메시지를 `output_`에 직접 출력 | TC-06 `InvalidInput_ShowsErrorMessage` 검증 가능 |
| Service 결과를 output에 직접 출력 | FS-11 View 레이어 없이도 동작 가능 |

### 의존성 처리

- `OrderService`, `ProductionService`는 실 구현체 참조로 주입한다.
- 테스트에서는 `InMemoryOrderRepository`, `InMemoryProductRepository` 기반
  실 객체를 사용하며, Mock은 사용하지 않는다.
- 테스트 Fixture 예시:

```cpp
class OrderControllerTest : public ::testing::Test {
protected:
    void SetUp() override {
        order_repo    = std::make_shared<repositories::InMemoryOrderRepository>();
        product_repo  = std::make_shared<repositories::InMemoryProductRepository>();
        order_svc     = std::make_unique<services::OrderService>(*order_repo, *product_repo);
        prod_svc      = std::make_unique<services::ProductionService>(*order_repo);
    }

    std::shared_ptr<repositories::InMemoryOrderRepository>   order_repo;
    std::shared_ptr<repositories::InMemoryProductRepository> product_repo;
    std::unique_ptr<services::OrderService>                  order_svc;
    std::unique_ptr<services::ProductionService>             prod_svc;
};
```

---

## 4. 테스트 케이스 목록

| # | 테스트 이름 | 검증 동작 | 입력 스트림 | 기대 결과 |
|---|------------|---------|------------|---------|
| 1 | `ParsesMenuInput_Correctly` | 메뉴 번호 0 파싱 시 정상 반환 | `"0\n"` | HandleInput이 예외 없이 반환 |
| 2 | `CreateOrder_CallsServiceWithParsedArgs` | 주문 생성 파싱 후 Service 호출 | `"1\n1\n100\n2099-12-31\n"` | `order_repo`에 Order 1건 저장됨 |
| 3 | `ConfirmOrder_CallsServiceWithOrderId` | 확정 파싱 후 Service 호출 | `"2\n{id}\n"` | `FindById(id)->status == CONFIRMED` |
| 4 | `RejectOrder_CallsServiceWithOrderId` | 반려 파싱 후 Service 호출 | `"3\n{id}\n"` | `FindById(id)->status == REJECTED` |
| 5 | `CancelOrder_CallsServiceWithOrderId` | 취소 파싱 후 Service 호출 | `"4\n{id}\n"` | `FindById(id)->status == REJECTED` |
| 6 | `InvalidInput_ShowsErrorMessage` | 잘못된 메뉴 입력 시 오류 문자열 출력 | `"99\n"` | `output.str()`에 오류 문자열 포함 |

> TC-3~5: Fixture SetUp에서 RESERVED 상태의 Order를 미리 저장한 뒤 id를 입력으로 사용한다.

---

## 5. stub 전략

RED 단계에서는 헤더 전용 inline stub으로 컴파일을 통과시킨 뒤 테스트 실패를 확인한다.

```cpp
// include/controllers/OrderController.h — inline stub 구현
inline OrderController::OrderController(
    services::OrderService&      order_service,
    services::ProductionService& production_service,
    std::istream&                input,
    std::ostream&                output)
    : order_service_(order_service)
    , production_service_(production_service)
    , input_(input)
    , output_(output) {}

inline void OrderController::HandleInput() {}  // stub: 아무 동작 없음

inline void OrderController::HandleCreateOrder()    {}
inline void OrderController::HandleConfirmOrder()   {}
inline void OrderController::HandleRejectOrder()    {}
inline void OrderController::HandleCancelOrder()    {}
inline void OrderController::HandleStartProduction(){}
inline void OrderController::HandleRelease()        {}
```

stub 상태에서의 예상 결과:

| # | 테스트 | stub 상태 결과 | 비고 |
|---|--------|-------------|------|
| 1 | `ParsesMenuInput_Correctly` | PASSED | 아무것도 안 해도 정상 반환 — 허용 |
| 2 | `CreateOrder_CallsServiceWithParsedArgs` | FAILED | repo에 Order 저장 안 됨 — RED 확인 대상 |
| 3 | `ConfirmOrder_CallsServiceWithOrderId` | FAILED | status 변경 없음 — RED 확인 대상 |
| 4 | `RejectOrder_CallsServiceWithOrderId` | FAILED | status 변경 없음 — RED 확인 대상 |
| 5 | `CancelOrder_CallsServiceWithOrderId` | FAILED | status 변경 없음 — RED 확인 대상 |
| 6 | `InvalidInput_ShowsErrorMessage` | FAILED | output에 아무것도 출력 안 됨 — RED 확인 대상 |

RED 단계 목표: 6개 테스트가 컴파일 성공 후 최소 5개(TC-2~TC-6) 실패함을 확인한다.

---

## 6. 파일 목록

| 역할 | 경로 |
|------|------|
| Controller 헤더 (신규) | `SampleOrderSystem-JH/include/controllers/OrderController.h` |
| 테스트 (신규) | `SampleOrderSystem-JH/tests/OrderControllerTest.cpp` |

---

## 7. 완료 기준

- [ ] RED: 6개 테스트 컴파일 성공 후 5개 이상 실패
- [ ] GREEN: 전체 테스트 PASSED (기존 53개 + 신규 6개 = 59개)
- [ ] 커밋①: GREEN 상태 feat 커밋
- [ ] REVIEW: 코드 리뷰 + 사용자 승인
- [ ] 커밋②: REFACTOR 반영 후 최종 커밋
