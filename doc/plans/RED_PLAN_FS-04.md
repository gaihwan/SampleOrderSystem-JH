# RED_PLAN_FS-04 — InMemoryOrderRepository

> 작성일: 2026-06-12  
> 브랜치: feat/fs-04-inmemory-order-repository  
> 연계: [PLAN.md §3 Phase 2 — FS-04](../../PLAN.md)

---

## 1. Slice 개요

| 항목 | 내용 |
|------|------|
| **Slice ID** | FS-04 |
| **레이어** | Repository |
| **핵심 책임** | Order 객체의 인메모리 CRUD 및 조회 필터링 |
| **의존 Slice** | FS-03 (models::Order, models::OrderStatus) |

---

## 2. PLAN.md 연계

```
| FS-04 | InMemoryOrderRepository | Repository | CRUD, FindByStatus | FS-03 |
```

---

## 3. 설계 결정

### 인터페이스 설계

```cpp
// include/repositories/IOrderRepository.h
namespace repositories {

class IOrderRepository {
public:
    virtual ~IOrderRepository() = default;
    virtual int               Save(const models::Order& order) = 0;
    virtual std::optional<models::Order>
                              FindById(int id) const = 0;
    virtual std::vector<models::Order>
                              FindAll() const = 0;
    virtual std::vector<models::Order>
                              FindByStatus(models::OrderStatus status) const = 0;
    virtual std::vector<models::Order>
                              FindByProductId(int product_id) const = 0;
    virtual bool              Update(const models::Order& order) = 0;
};

}  // namespace repositories
```

```cpp
// include/repositories/InMemoryOrderRepository.h
namespace repositories {

class InMemoryOrderRepository : public IOrderRepository {
public:
    int  Save(const models::Order& order) override;
    std::optional<models::Order> FindById(int id) const override;
    std::vector<models::Order>   FindAll() const override;
    std::vector<models::Order>   FindByStatus(models::OrderStatus status) const override;
    std::vector<models::Order>   FindByProductId(int product_id) const override;
    bool Update(const models::Order& order) override;
private:
    std::unordered_map<int, models::Order> store_;
    int next_id_ = 1;
};

}  // namespace repositories
```

### 설계 결정 사항

| 결정 | 이유 |
|------|------|
| `std::optional<Order>` 반환 | 없는 ID 조회 시 nullptr 대신 타입 안전한 빈값 표현 (C++17) |
| `Save` 가 `int` 반환 | 자동 생성된 ID를 호출자에게 전달 |
| `std::unordered_map<int, Order>` | O(1) ID 조회, 순서 무관 |
| `Update` 가 `bool` 반환 | 존재하지 않는 ID 업데이트 시 false 반환으로 오류 전달 |
| 인터페이스(IOrderRepository) 분리 | Service 레이어가 구현체에 직접 의존하지 않도록 DI 설계 |

### 의존성 처리

- `models::Order`, `models::OrderStatus` — FS-03 완료로 사용 가능
- 외부 의존성 없음 → Mock 불필요, 실제 객체로 테스트

---

## 4. 테스트 케이스 목록

| # | 테스트 이름 | 검증 동작 | 입력 | 기대 출력 |
|---|------------|---------|------|---------|
| 1 | `Save_AssignsAutoIncrementId` | 저장 시 자동 증가 ID 부여 | Order (id=0) 2개 순차 저장 | 첫 번째 id=1, 두 번째 id=2 |
| 2 | `FindById_ReturnsOrder_WhenExists` | 존재하는 ID 조회 | id=1 저장 후 FindById(1) | Order 반환, has_value=true |
| 3 | `FindById_ReturnsNullopt_WhenNotFound` | 없는 ID 조회 | FindById(999) | std::nullopt |
| 4 | `FindAll_ReturnsAllOrders` | 전체 조회 | Order 3개 저장 후 FindAll() | size=3 |
| 5 | `FindByStatus_FiltersCorrectly` | 상태별 필터 | RESERVED 2개 + CONFIRMED 1개 저장 | FindByStatus(RESERVED) → size=2 |
| 6 | `Update_ChangesOrderStatus` | 상태 변경 | Save 후 status=CONFIRMED으로 Update | FindById 후 status==CONFIRMED |
| 7 | `FindByProductId_ReturnsMatchingOrders` | product_id 필터 | product_id=1 × 2개, product_id=2 × 1개 | FindByProductId(1) → size=2 |

---

## 5. stub 전략

`InMemoryOrderRepository`의 모든 메서드를 **빈 구현**으로 작성한다:
- `Save` → `return 0;`
- `FindById` → `return std::nullopt;`
- `FindAll` / `FindByStatus` / `FindByProductId` → `return {};`
- `Update` → `return false;`

이 상태에서 테스트가 컴파일 성공 후 실패함을 확인한다.

---

## 6. 파일 목록

| 역할 | 경로 |
|------|------|
| Repository 인터페이스 | `SampleOrderSystem-JH/include/repositories/IOrderRepository.h` |
| InMemory 구현체 (stub→구현) | `SampleOrderSystem-JH/include/repositories/InMemoryOrderRepository.h` |
| 테스트 | `SampleOrderSystem-JH/tests/InMemoryOrderRepositoryTest.cpp` |

---

## 7. 완료 기준

- [ ] RED: 7개 테스트 컴파일 성공 후 실패
- [ ] GREEN: 전체 테스트 PASSED (기존 18개 + 신규 7개 = 25개)
- [ ] 커밋①: GREEN 상태 feat 커밋
- [ ] REVIEW: 코드 리뷰 + 사용자 승인
- [ ] 커밋②: REFACTOR 반영 후 최종 커밋
