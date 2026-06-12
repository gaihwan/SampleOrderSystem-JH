# RED_PLAN_FS-11 — OrderView / ProductionView 콘솔 출력 포맷

> 작성일: 2026-06-12  
> 브랜치: feat/fs-11-order-view  
> 연계: [PLAN.md §3 Phase 5 — FS-11](../../PLAN.md)

---

## 1. Slice 개요

| 항목 | 내용 |
|------|------|
| **Slice ID** | FS-11 |
| **레이어** | View |
| **핵심 책임** | 주문 목록 출력 형식, 생산 현황 출력 형식 |
| **의존 Slice** | FS-03 (Order/Product 모델) |

---

## 2. PLAN.md 연계

> PLAN.md Feature Slice 목록 중 해당 항목:

```
| FS-11 | OrderView / ProductionView | View | 주문 목록 출력, 생산 현황 출력 | FS-03 |
```

---

## 3. 설계 결정

### 핵심 설계 원칙 — `std::ostream` 주입으로 테스트 가능하게

`std::ostream&` 참조를 생성자에서 주입받아 테스트에서 `std::ostringstream`으로
출력을 캡처할 수 있게 설계한다. 외부 의존성이 없으므로 Mock은 사용하지 않는다.

### 인터페이스 설계

```cpp
// SampleOrderSystem-JH/include/views/OrderView.h
#pragma once
#include "models/Order.h"
#include <ostream>
#include <vector>
#include <string>

namespace views {

class OrderView {
public:
    explicit OrderView(std::ostream& output);

    // 주문 목록을 테이블 형식으로 출력
    // show_rejected=false이면 REJECTED 상태 주문 제외
    void RenderOrderList(const std::vector<models::Order>& orders,
                         bool show_rejected = true) const;

private:
    std::ostream& output_;

    static std::string StatusToString(models::OrderStatus status);
};

}  // namespace views
```

```cpp
// SampleOrderSystem-JH/include/views/ProductionView.h
#pragma once
#include "models/Order.h"
#include "models/Product.h"
#include <ostream>
#include <vector>

namespace views {

class ProductionView {
public:
    explicit ProductionView(std::ostream& output);

    // 생산 현황을 테이블 형식으로 출력
    // products: 시료명 조회에 사용
    void RenderProductionStatus(const std::vector<models::Order>& orders,
                                const std::vector<models::Product>& products) const;

private:
    std::ostream& output_;
};

}  // namespace views
```

### 출력 포맷

**OrderView::RenderOrderList:**
```
[주문 목록]
ID  | 시료ID | 수량  | 납기일     | 상태      | 배치수 | 예상수율
-----------------------------------------------------------------
1   | 2      | 100   | 2099-12-31 | RESERVED  | 3      | 135.0
2   | 3      | 200   | 2099-06-30 | CONFIRMED | 5      | 225.0
```

**ProductionView::RenderProductionStatus:**
```
[생산 현황]
ID  | 시료명        | 수량  | 상태      | 배치수
-------------------------------------------------
1   | TestSample    | 100   | PRODUCING | 3
```

### 설계 결정 사항

| 결정 | 이유 |
|------|------|
| `std::ostream&` 주입 | `ostringstream`으로 테스트 출력 캡처 가능 |
| `show_rejected` 파라미터 | TC-03 필터 테스트 지원 |
| `RenderProductionStatus`에 `products` 파라미터 | 시료명(Product::name) 조회 위해 |
| 테이블 형식 출력 | 콘솔 가독성 |

### 의존성 처리

- 테스트에서 `models::Order`, `models::Product`를 직접 생성하여 주입한다.
- 외부 Service/Repository 의존성 없음, Mock 불필요.
- `Order.h`, `Product.h`(FS-03)만 include한다.

---

## 4. 테스트 케이스 목록

| # | 테스트 이름 | 검증 동작 | 입력 | 기대 출력 포함 문자열 |
|---|------------|---------|------|---------------------|
| 1 | `OrderViewTest_RenderOrderList_ShowsAllColumns` | 헤더 및 주문 1건의 모든 컬럼 출력 | `Order{id=1, product_id=2, quantity=100, deadline="2099-12-31", status=RESERVED, required_batches=3, estimated_yield=135.0}` | `"ID"`, `"수량"`, `"1"`, `"100"` |
| 2 | `OrderViewTest_RenderOrderList_ShowsDeadlineAndBatch_WhenConfirmed` | 납기일과 배치수 포함 출력 | CONFIRMED Order (deadline="2099-06-30", required_batches=5) | `"2099-06-30"`, `"5"` |
| 3 | `OrderViewTest_RenderOrderList_FiltersRejectedOrders_WhenRequested` | show_rejected=false 시 REJECTED 주문 제외 | REJECTED Order(id=1) + RESERVED Order(id=2), show_rejected=false | `"2"` 포함, `"1"` 미포함(REJECTED) |
| 4 | `ProductionViewTest_RenderProductionStatus_ShowsBatchProgress` | 생산 현황 헤더 및 배치수 출력 | PRODUCING Order (required_batches=3), Product(name="TestSample") | `"생산 현황"`, `"3"` |
| 5 | `ProductionViewTest_RenderProductionStatus_ShowsEstimatedCompletionDate` | estimated_yield 값 포함 출력 | PRODUCING Order (estimated_yield=135.0), Product(name="TestSample") | `"135"` (estimated_yield의 문자열 표현) |

> TC-03: REJECTED(id=1)와 RESERVED(id=2) 2건을 동시에 전달하여 RESERVED만 출력되는지 확인한다.

---

## 5. stub 전략

RED 단계에서는 헤더 전용 inline stub으로 컴파일을 통과시킨 뒤 테스트 실패를 확인한다.

```cpp
// OrderView inline stub
inline OrderView::OrderView(std::ostream& output) : output_(output) {}
inline void OrderView::RenderOrderList(
    const std::vector<models::Order>&, bool) const {}  // 아무것도 출력하지 않음

// ProductionView inline stub
inline ProductionView::ProductionView(std::ostream& output) : output_(output) {}
inline void ProductionView::RenderProductionStatus(
    const std::vector<models::Order>&,
    const std::vector<models::Product>&) const {}  // 아무것도 출력하지 않음
```

stub 상태에서의 예상 결과:

| # | 테스트 | stub 상태 결과 | 비고 |
|---|--------|-------------|------|
| 1 | `RenderOrderList_ShowsAllColumns` | FAILED | output이 비어 있어 "ID" 미포함 — RED 확인 대상 |
| 2 | `RenderOrderList_ShowsDeadlineAndBatch_WhenConfirmed` | FAILED | output이 비어 있어 납기일 미포함 — RED 확인 대상 |
| 3 | `RenderOrderList_FiltersRejectedOrders_WhenRequested` | FAILED | output이 비어 있어 RESERVED id도 미포함 — RED 확인 대상 |
| 4 | `RenderProductionStatus_ShowsBatchProgress` | FAILED | output이 비어 있어 "생산 현황" 미포함 — RED 확인 대상 |
| 5 | `RenderProductionStatus_ShowsEstimatedCompletionDate` | FAILED | output이 비어 있어 estimated_yield 미포함 — RED 확인 대상 |

RED 단계 목표: 5개 테스트 모두 컴파일 성공 후 전부 FAILED 확인.

---

## 6. 파일 목록

| 역할 | 경로 |
|------|------|
| OrderView 헤더 (신규) | `SampleOrderSystem-JH/include/views/OrderView.h` |
| ProductionView 헤더 (신규) | `SampleOrderSystem-JH/include/views/ProductionView.h` |
| 테스트 (신규) | `SampleOrderSystem-JH/tests/OrderViewTest.cpp` |
| 테스트 (신규) | `SampleOrderSystem-JH/tests/ProductionViewTest.cpp` |

---

## 7. 완료 기준

- [ ] RED: 5개 테스트 컴파일 성공 후 전부 실패 (기존 61개 유지)
- [ ] GREEN: 전체 테스트 PASSED (기존 61개 + 신규 5개 = 66개)
- [ ] 커밋①: GREEN 상태 feat 커밋
- [ ] REVIEW: 코드 리뷰 + 사용자 승인
- [ ] 커밋②: REFACTOR 반영 후 최종 커밋
