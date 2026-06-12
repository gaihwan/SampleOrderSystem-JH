# RED_PLAN_FS-05 — InMemoryProductRepository

> 작성일: 2026-06-12  
> 브랜치: feat/fs-05-inmemory-product-repository  
> 연계: [PLAN.md §3 Phase 2 — FS-05](../../PLAN.md)

---

## 1. Slice 개요

| 항목 | 내용 |
|------|------|
| **Slice ID** | FS-05 |
| **레이어** | Repository |
| **핵심 책임** | Product 객체의 인메모리 CRUD 및 조회 |
| **의존 Slice** | FS-03 (models::Product) |

---

## 2. PLAN.md 연계

```
| FS-05 | InMemoryProductRepository | Repository | CRUD, FindById | FS-03 |
```

---

## 3. 설계 결정

### 인터페이스 설계

```cpp
// include/repositories/IProductRepository.h
namespace repositories {

class IProductRepository {
public:
    virtual ~IProductRepository() = default;
    [[nodiscard]] virtual int                             Save(const models::Product& product) = 0;
    [[nodiscard]] virtual std::optional<models::Product> FindById(int id) const = 0;
    [[nodiscard]] virtual std::vector<models::Product>   FindAll() const = 0;
    virtual bool                                         Update(const models::Product& product) = 0;
};

}  // namespace repositories
```

```cpp
// include/repositories/InMemoryProductRepository.h
namespace repositories {

// Not thread-safe. Intended for single-threaded console use only.
class InMemoryProductRepository : public IProductRepository {
public:
    int                                    Save(const models::Product& product) override;
    std::optional<models::Product> FindById(int id) const noexcept override;
    std::vector<models::Product>   FindAll() const noexcept override;
    bool                           Update(const models::Product& product) noexcept override;
private:
    std::unordered_map<int, models::Product> store_;
    int next_id_ = 1;
};

}  // namespace repositories
```

### 설계 결정 사항

| 결정 | 이유 |
|------|------|
| `std::optional<Product>` 반환 | 없는 ID 조회 시 nullptr 대신 타입 안전한 빈값 표현 (C++17) |
| `Save` 가 `int` 반환 | 자동 생성된 ID를 호출자에게 전달 |
| `std::unordered_map<int, Product>` | O(1) ID 조회, 순서 무관 |
| `Update` 가 `bool` 반환 | 존재하지 않는 ID 업데이트 시 false 반환으로 오류 전달 |
| 인터페이스(IProductRepository) 분리 | Service 레이어가 구현체에 직접 의존하지 않도록 DI 설계 |

### 의존성 처리

- `models::Product` — FS-03 완료로 사용 가능
- 외부 의존성 없음 → Mock 불필요, 실제 객체로 테스트

---

## 4. 테스트 케이스 목록

| # | 테스트 이름 | 검증 동작 | 입력 | 기대 출력 |
|---|------------|---------|------|---------|
| 1 | `Save_StoresProduct` | Save 후 FindById로 조회 성공 | Product 저장 후 FindById(반환 id) | has_value=true, 필드 일치 |
| 2 | `FindById_ReturnsProduct_WhenExists` | 존재하는 ID 조회 | Product 저장 후 FindById(반환 id) | Product 반환, has_value=true |
| 3 | `FindById_ReturnsNullopt_WhenNotFound` | 없는 ID 조회 | FindById(999) | std::nullopt |
| 4 | `FindAll_ReturnsAllProducts` | 전체 조회 | Product 3개 저장 후 FindAll() | size=3 |

---

## 5. stub 전략

`InMemoryProductRepository`의 모든 메서드를 **빈 구현**으로 작성한다:

- `Save` → `return 0;`
- `FindById` → `return std::nullopt;`
- `FindAll` → `return {};`
- `Update` → `return false;`

이 상태에서 테스트가 컴파일 성공 후 실패함을 확인한다.

---

## 6. 파일 목록

| 역할 | 경로 |
|------|------|
| Repository 인터페이스 | `SampleOrderSystem-JH/include/repositories/IProductRepository.h` |
| InMemory 구현체 (stub→구현) | `SampleOrderSystem-JH/include/repositories/InMemoryProductRepository.h` |
| 테스트 | `SampleOrderSystem-JH/tests/InMemoryProductRepositoryTest.cpp` |

---

## 7. 완료 기준

- [ ] RED: 4개 테스트 컴파일 성공 후 실패
- [ ] GREEN: 전체 테스트 PASSED (기존 25개 + 신규 4개 = 29개)
- [ ] 커밋①: GREEN 상태 feat 커밋
- [ ] REVIEW: 코드 리뷰 + 사용자 승인
- [ ] 커밋②: REFACTOR 반영 후 최종 커밋
