# RED_PLAN_FS-07 — OrderService 상태 전이

> 작성일: 2026-06-12  
> 브랜치: feat/fs-07-order-service-transition  
> 연계: [PLAN.md §3 Phase 3 — FS-07](../../PLAN.md)

---

## 1. Slice 개요

| 항목 | 내용 |
|------|------|
| **Slice ID** | FS-07 |
| **레이어** | Service |
| **핵심 책임** | Order 상태 전이 규칙 검증 및 적용 (confirm / reject / cancel) |
| **의존 Slice** | FS-06 (OrderService 기반), FS-04 (IOrderRepository) |

---

## 2. PLAN.md 연계

```
| FS-07 | OrderService | Service | 상태 전이 ConfirmOrder·RejectOrder·CancelOrder | FS-06, FS-04 |
```

---

## 3. 설계 결정

### 인터페이스 설계

```cpp
// include/services/OrderService.h — FS-07에서 추가되는 메서드
namespace services {

class OrderService {
public:
    // (FS-06 기존)
    [[nodiscard]] ServiceResult CreateOrder(const CreateOrderRequest& req);

    // (FS-07 신규)
    [[nodiscard]] ServiceResult ConfirmOrder(int order_id);
    [[nodiscard]] ServiceResult RejectOrder(int order_id);
    [[nodiscard]] ServiceResult CancelOrder(int order_id);
};

}  // namespace services
```

### 상태 전이 규칙표

| 메서드 | 허용 현재 상태 | 결과 상태 | 실패 조건 |
|--------|--------------|---------|---------|
| `ConfirmOrder` | RESERVED | CONFIRMED | 다른 상태, ID 없음 |
| `RejectOrder` | RESERVED, CONFIRMED | REJECTED | 다른 상태, ID 없음 |
| `CancelOrder` | RESERVED, CONFIRMED, PRODUCING, RELEASE | REJECTED | 이미 REJECTED, ID 없음 |

### 설계 결정 사항

| 결정 | 이유 |
|------|------|
| 기존 `ServiceResult` 재사용 | 성공/실패·오류 메시지 패턴이 CreateOrder와 동일 |
| `FindById` + `Update` 조합으로 구현 | IOrderRepository 인터페이스 변경 없이 활용 가능 |
| ID 없음과 상태 오류를 구분된 error_message로 반환 | 호출자가 오류 유형을 판별할 수 있도록 함 |

### 의존성 처리

- `InMemoryOrderRepository` 실 구현체를 사용한다 (Mock 불필요)
- FS-06 `OrderServiceCreateTest` 픽스처 패턴을 재사용한다:
  - `InMemoryOrderRepository` + `InMemoryProductRepository` 인스턴스 직접 생성
  - `OrderService` 생성자에 주입
  - 테스트별로 `CreateOrder`로 초기 주문을 생성한 뒤 상태 전이 메서드 호출

---

## 4. 테스트 케이스 목록

| # | 테스트 이름 | 검증 동작 | 입력 | 기대 출력 |
|---|------------|---------|------|---------|
| 1 | `ConfirmOrder_ChangesStatus_ToConfirmed` | RESERVED → CONFIRMED 전이 | CreateOrder 후 ConfirmOrder(order_id) | `FindById(order_id)->status == CONFIRMED` |
| 2 | `ConfirmOrder_Fails_WhenStatusIsNotReserved` | 비RESERVED 상태 거부 | ConfirmOrder 후 다시 ConfirmOrder | `success == false` |
| 3 | `RejectOrder_ChangesStatus_ToRejected_FromReserved` | RESERVED → REJECTED 전이 | CreateOrder 후 RejectOrder(order_id) | `FindById(order_id)->status == REJECTED` |
| 4 | `RejectOrder_ChangesStatus_ToRejected_FromConfirmed` | CONFIRMED → REJECTED 전이 | ConfirmOrder 후 RejectOrder(order_id) | `FindById(order_id)->status == REJECTED` |
| 5 | `CancelOrder_ChangesStatus_ToRejected_FromAnyNonRejectedStatus` | 임의 비거부 상태 → REJECTED | CreateOrder 후 CancelOrder(order_id) | `FindById(order_id)->status == REJECTED` |
| 6 | `CancelOrder_Fails_WhenAlreadyRejected` | 이미 REJECTED 상태 거부 | RejectOrder 후 CancelOrder(order_id) | `success == false` |
| 7 | `CancelOrder_Fails_WhenOrderNotFound` | 존재하지 않는 ID 처리 | CancelOrder(999) | `success == false` |

---

## 5. stub 전략

`OrderService` 헤더에 세 메서드를 선언하고, 구현 파일에 빈 stub을 작성한다:

```cpp
// src/services/OrderService.cpp — FS-07 stub 추가분
ServiceResult OrderService::ConfirmOrder(int order_id) {
    return ServiceResult{};  // stub: success=false
}

ServiceResult OrderService::RejectOrder(int order_id) {
    return ServiceResult{};  // stub: success=false
}

ServiceResult OrderService::CancelOrder(int order_id) {
    return ServiceResult{};  // stub: success=false
}
```

stub(success=false) 상태에서의 테스트 결과:

| 테스트 | stub 상태 결과 | 비고 |
|--------|-------------|------|
| 1 ConfirmOrder_ChangesStatus_ToConfirmed | FAILED | status 기대값 불일치 — RED 확인 대상 |
| 2 ConfirmOrder_Fails_WhenStatusIsNotReserved | PASSED | success==false 우연 일치 — 허용 |
| 3 RejectOrder_ChangesStatus_ToRejected_FromReserved | FAILED | status 기대값 불일치 — RED 확인 대상 |
| 4 RejectOrder_ChangesStatus_ToRejected_FromConfirmed | FAILED | status 기대값 불일치 — RED 확인 대상 |
| 5 CancelOrder_ChangesStatus_ToRejected_FromAnyNonRejectedStatus | FAILED | status 기대값 불일치 — RED 확인 대상 |
| 6 CancelOrder_Fails_WhenAlreadyRejected | PASSED | success==false 우연 일치 — 허용 |
| 7 CancelOrder_Fails_WhenOrderNotFound | PASSED | success==false 우연 일치 — 허용 |

RED 단계 목표: 7개 테스트가 컴파일 성공 후 최소 4개 실패함을 확인한다.

---

## 6. 파일 목록

| 역할 | 경로 |
|------|------|
| Service 헤더 (기존 수정 — 메서드 선언 추가) | `SampleOrderSystem-JH/include/services/OrderService.h` |
| Service 구현 (stub 추가) | `SampleOrderSystem-JH/src/services/OrderService.cpp` |
| 테스트 (신규) | `SampleOrderSystem-JH/tests/OrderServiceTransitionTest.cpp` |

---

## 7. 완료 기준

- [ ] RED: 7개 테스트 컴파일 성공 후 4개 이상 실패
- [ ] GREEN: 전체 테스트 PASSED (기존 35개 + 신규 7개 = 42개)
- [ ] 커밋①: GREEN 상태 feat 커밋
- [ ] REVIEW: 코드 리뷰 + 사용자 승인
- [ ] 커밋②: REFACTOR 반영 후 최종 커밋
