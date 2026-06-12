# RED_PLAN_FS-08 — ProductionService 생산 관리

> 작성일: 2026-06-12  
> 브랜치: feat/fs-08-production-service  
> 연계: [PLAN.md §3 Phase 3 — FS-08](../../PLAN.md)

---

## 1. Slice 개요

| 항목 | 내용 |
|------|------|
| **Slice ID** | FS-08 |
| **레이어** | Service |
| **핵심 책임** | 생산 시작(FIFO 강제), 릴리즈, 생산 현황 조회 |
| **의존 Slice** | FS-07 (OrderService), FS-04 (IOrderRepository) |

---

## 2. PLAN.md 연계

```
| FS-08 | ProductionService | Service | StartProduction·Release·GetProductionStatus | FS-07, FS-04 |
```

---

## 3. 설계 결정

### 인터페이스 설계

```cpp
// include/services/ProductionService.h
namespace services {

class ProductionService {
public:
    explicit ProductionService(repositories::IOrderRepository& order_repo);

    [[nodiscard]] ServiceResult              StartProduction(int order_id) noexcept;
    [[nodiscard]] ServiceResult              Release(int order_id) noexcept;
    [[nodiscard]] std::vector<models::Order> GetProductionStatus() const noexcept;

private:
    repositories::IOrderRepository& order_repo_;
    // Helper
    std::optional<models::Order> FindOrder(int order_id, ServiceResult& err) const noexcept;
};

}  // namespace services
```

### FIFO 규칙 구현 방법

```
StartProduction(order_id):
1. FindById(order_id) → 없으면 실패
2. status != CONFIRMED → 실패
3. FindByStatus(CONFIRMED) 전체 조회
4. min_id = 모든 CONFIRMED 주문 중 가장 작은 id
5. order_id != min_id → "FIFO 순서 위반: 먼저 생성된 주문을 먼저 생산해야 합니다" 실패
6. status = PRODUCING, Update, 성공 반환
```

### 설계 결정 사항

| 결정 | 이유 |
|------|------|
| `ServiceResult` 재사용 (services 네임스페이스 공유) | FS-06/07과 동일한 성공/실패 패턴 |
| `IOrderRepository`만 주입 | ProductionService는 Product 정보 불필요 |
| FIFO 구현: `FindAll()` + min id 탐색 | Repository에 정렬 쿼리 추가 없이 단순하게 |
| `GetProductionStatus`: FindByStatus 두 번 호출 | CONFIRMED + PRODUCING 합산 |

### 의존성 처리

- `InMemoryOrderRepository` 실 구현체 사용 (Mock 불필요)
- 테스트에서 OrderService를 사용해 주문 생성(RESERVED) 후 ConfirmOrder로 CONFIRMED 전이
- ProductionService는 별도 인스턴스로 동일한 `order_repo_` 참조를 공유

---

## 4. 테스트 케이스 목록

| # | 테스트 이름 | 검증 동작 | 입력 | 기대 출력 |
|---|------------|---------|------|---------|
| 1 | `StartProduction_ChangesStatus_ToProducing` | CONFIRMED → PRODUCING 전이 | Confirm 후 StartProduction(order_id) | `FindById(order_id)->status == PRODUCING` |
| 2 | `StartProduction_Fails_WhenStatusIsNotConfirmed` | 비CONFIRMED 상태 거부 | RESERVED 주문에 StartProduction | `success == false` |
| 3 | `StartProduction_FollowsFIFO_Order` | FIFO 순서 강제 | id=1, id=2 Confirm 후 id=2 먼저 StartProduction | `success == false` |
| 4 | `Release_ChangesStatus_ToRelease` | PRODUCING → RELEASE 전이 | StartProduction 후 Release(order_id) | `FindById(order_id)->status == RELEASE` |
| 5 | `Release_Fails_WhenStatusIsNotProducing` | 비PRODUCING 상태 거부 | CONFIRMED 주문에 Release | `success == false` |
| 6 | `GetProductionStatus_ReturnsConfirmedAndProducingOrders` | CONFIRMED+PRODUCING 반환 | CONFIRMED 1개 + PRODUCING 1개 + RESERVED 1개 | `result.size() == 2` |

---

## 5. stub 전략

RED 단계에서는 헤더 전용 inline stub으로 컴파일을 통과시킨 뒤 테스트 실패를 확인한다.

```cpp
// include/services/ProductionService.h — inline stub 구현
inline ServiceResult ProductionService::StartProduction(int /*order_id*/) noexcept {
    return ServiceResult{};  // stub: success=false
}

inline ServiceResult ProductionService::Release(int /*order_id*/) noexcept {
    return ServiceResult{};  // stub: success=false
}

inline std::vector<models::Order> ProductionService::GetProductionStatus() const noexcept {
    return {};  // stub: 빈 벡터
}
```

stub(success=false, 빈 벡터) 상태에서의 예상 결과:

| # | 테스트 | stub 상태 결과 | 비고 |
|---|--------|-------------|------|
| 1 | `StartProduction_ChangesStatus_ToProducing` | FAILED | status 기대값 불일치 — RED 확인 대상 |
| 2 | `StartProduction_Fails_WhenStatusIsNotConfirmed` | PASSED | success==false 우연 일치 — 허용 |
| 3 | `StartProduction_FollowsFIFO_Order` | PASSED | success==false 우연 일치 — 허용 |
| 4 | `Release_ChangesStatus_ToRelease` | FAILED | status 기대값 불일치 — RED 확인 대상 |
| 5 | `Release_Fails_WhenStatusIsNotProducing` | PASSED | success==false 우연 일치 — 허용 |
| 6 | `GetProductionStatus_ReturnsConfirmedAndProducingOrders` | FAILED | size==2 기대인데 0 — RED 확인 대상 |

RED 단계 목표: 6개 테스트가 컴파일 성공 후 최소 3개(TC-1, TC-4, TC-6) 실패함을 확인한다.

---

## 6. 파일 목록

| 역할 | 경로 |
|------|------|
| Service 헤더 (신규) | `SampleOrderSystem-JH/include/services/ProductionService.h` |
| 테스트 (신규) | `SampleOrderSystem-JH/tests/ProductionServiceTest.cpp` |

---

## 7. 완료 기준

- [ ] RED: 6개 테스트 컴파일 성공 후 3개 이상 실패
- [ ] GREEN: 전체 테스트 PASSED (기존 42개 + 신규 6개 = 48개)
- [ ] 커밋①: GREEN 상태 feat 커밋
- [ ] REVIEW: 코드 리뷰 + 사용자 승인
- [ ] 커밋②: REFACTOR 반영 후 최종 커밋
