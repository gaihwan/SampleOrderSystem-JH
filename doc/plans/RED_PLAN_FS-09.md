# RED_PLAN_FS-09 — FileOrderRepository 파일 기반 영속화

> 작성일: 2026-06-12  
> 브랜치: feat/fs-09-file-order-repository  
> 연계: [PLAN.md §3 Phase 4 — FS-09](../../PLAN.md)

---

## 1. Slice 개요

| 항목 | 내용 |
|------|------|
| **Slice ID** | FS-09 |
| **레이어** | Repository |
| **핵심 책임** | Order CRUD를 JSON 파일로 영속화 (재시작 후 복원) |
| **의존 Slice** | FS-03 (Order 모델), FS-04 (IOrderRepository 인터페이스) |

---

## 2. PLAN.md 연계

```
| FS-09 | FileOrderRepository | Repository | Save·FindById·FindAll·FindByStatus·FindByProductId·Update (파일 영속) | FS-03, FS-04 |
```

---

## 3. 설계 결정

### 인터페이스 설계

```cpp
// include/repositories/FileOrderRepository.h
namespace repositories {

class FileOrderRepository : public IOrderRepository {
public:
    explicit FileOrderRepository(const std::string& file_path);

    [[nodiscard]] int                          Save(const models::Order& order) override;
    [[nodiscard]] std::optional<models::Order> FindById(int id) const noexcept override;
    [[nodiscard]] std::vector<models::Order>   FindAll() const noexcept override;
    [[nodiscard]] std::vector<models::Order>   FindByStatus(models::OrderStatus status) const noexcept override;
    [[nodiscard]] std::vector<models::Order>   FindByProductId(int product_id) const noexcept override;
    [[nodiscard]] bool                         Update(const models::Order& order) noexcept override;

private:
    std::string file_path_;
    int         next_id_ = 1;

    void                       SaveToFile(const std::vector<models::Order>& orders) const;
    std::vector<models::Order> LoadFromFile() const;
    static std::string         OrderToJson(const models::Order& order);
    static models::Order       JsonToOrder(const std::string& json_line);
    static std::string         StatusToString(models::OrderStatus status);
    static models::OrderStatus StringToStatus(const std::string& s);
};

}  // namespace repositories
```

### JSON 직렬화 포맷 — JSONL (newline-delimited, 1줄 1주문)

```json
{"id":1,"product_id":2,"quantity":100,"deadline":"2099-12-31","status":"RESERVED","required_batches":3,"estimated_yield":135.0,"created_at":""}
{"id":2,"product_id":3,"quantity":200,"deadline":"2099-06-30","status":"CONFIRMED","required_batches":5,"estimated_yield":225.0,"created_at":""}
```

### 설계 결정 사항

| 결정 | 이유 |
|------|------|
| JSONL (줄 단위 JSON) | 파싱 라이브러리 없이 `std::getline` + 수동 파싱 가능 |
| Save 시 파일 전체 재기록 | 단순성 우선, 데이터 소량 가정 |
| `next_id_` 를 LoadFromFile()로 복원 | 재시작 후 ID 충돌 방지 |
| 헤더 전용 inline 구현 | 프로젝트 기존 패턴 유지 |
| 테스트용 임시 파일 경로 주입 | 테스트 격리 (실제 파일 오염 방지) |

### 수동 JSON 파싱 전략 (외부 라이브러리 없음)

- `OrderToJson`: `std::string` 조합으로 JSON 직렬화
- `JsonToOrder`: `std::string::find` + `substr`로 필드 추출
  - 필드 순서가 고정되어 있으므로 순서 기반 파싱 가능
  - 문자열 필드는 `"` 사이 값 추출
  - 숫자 필드는 `std::stoi` / `std::stod` 변환

### 의존성 처리

- 외부 의존성 없음: `IOrderRepository` 인터페이스 구현체
- 테스트에서 임시 파일 경로(`test_orders_temp.json`)를 생성자에 주입
- `TearDown`에서 `std::remove`로 파일 삭제하여 테스트 격리 보장

```cpp
class FileOrderRepositoryTest : public ::testing::Test {
protected:
    const std::string test_file_ = "test_orders_temp.json";
    void TearDown() override {
        std::remove(test_file_.c_str());  // 테스트 후 파일 삭제
    }
};
```

---

## 4. 테스트 케이스 목록

| # | 테스트 이름 | 검증 동작 | 입력 | 기대 출력 |
|---|------------|---------|------|---------|
| 1 | `Save_WritesJsonFile` | Save 후 파일 존재 + FindAll size=1 | Order 1개 저장 | `FindAll().size() == 1` |
| 2 | `FindAll_ReadsFromJsonFile` | 저장 후 FindAll로 내용 검증 | Order 저장 후 FindAll | 동일 Order 반환 (product_id, quantity 일치) |
| 3 | `PersistsAcrossReload` | 새 인스턴스로 재로드 시 데이터 동일 | Save 후 새 `FileOrderRepository` 생성 | `FindAll().size()==1`, 필드 일치 |
| 4 | `Update_OverwritesExistingEntry` | Update 후 FindById 결과 반영 | Save 후 status 변경 Update | `FindById` 결과 status 일치 |
| 5 | `HandlesEmptyFile_GracefullyReturnsEmpty` | 빈 파일/미존재 파일 처리 | 빈 경로로 FindAll | `size()==0`, 예외 없음 |

---

## 5. stub 전략

RED 단계에서는 헤더 전용 inline stub으로 컴파일을 통과시킨 뒤 테스트 실패를 확인한다.

```cpp
// include/repositories/FileOrderRepository.h — inline stub 구현
inline int FileOrderRepository::Save(const models::Order& /*order*/) {
    return 0;  // stub: 저장하지 않음
}

inline std::optional<models::Order> FileOrderRepository::FindById(int /*id*/) const noexcept {
    return std::nullopt;  // stub: 항상 없음
}

inline std::vector<models::Order> FileOrderRepository::FindAll() const noexcept {
    return {};  // stub: 빈 벡터
}

inline std::vector<models::Order> FileOrderRepository::FindByStatus(models::OrderStatus) const noexcept {
    return {};  // stub: 빈 벡터
}

inline std::vector<models::Order> FileOrderRepository::FindByProductId(int) const noexcept {
    return {};  // stub: 빈 벡터
}

inline bool FileOrderRepository::Update(const models::Order&) noexcept {
    return false;  // stub: 항상 실패
}

// private helpers — 빈 구현
inline void FileOrderRepository::SaveToFile(const std::vector<models::Order>&) const {}
inline std::vector<models::Order> FileOrderRepository::LoadFromFile() const { return {}; }
inline std::string FileOrderRepository::OrderToJson(const models::Order&) { return ""; }
inline models::Order FileOrderRepository::JsonToOrder(const std::string&) { return {}; }
inline std::string FileOrderRepository::StatusToString(models::OrderStatus) { return ""; }
inline models::OrderStatus FileOrderRepository::StringToStatus(const std::string&) { return models::OrderStatus::RESERVED; }
```

stub 상태에서의 예상 결과:

| # | 테스트 | stub 상태 결과 | 비고 |
|---|--------|-------------|------|
| 1 | `Save_WritesJsonFile` | FAILED | FindAll().size()==0, 기대 1 — RED 확인 대상 |
| 2 | `FindAll_ReadsFromJsonFile` | FAILED | 빈 벡터 반환 — RED 확인 대상 |
| 3 | `PersistsAcrossReload` | FAILED | 재로드 후 size==0 — RED 확인 대상 |
| 4 | `Update_OverwritesExistingEntry` | FAILED | FindById nullopt 반환 — RED 확인 대상 |
| 5 | `HandlesEmptyFile_GracefullyReturnsEmpty` | PASSED | size()==0 우연 일치 — 허용 |

RED 단계 목표: 5개 테스트가 컴파일 성공 후 최소 4개(TC-1~TC-4) 실패함을 확인한다.

---

## 6. 파일 목록

| 역할 | 경로 |
|------|------|
| Repository 헤더 (신규) | `SampleOrderSystem-JH/include/repositories/FileOrderRepository.h` |
| 테스트 (신규) | `SampleOrderSystem-JH/tests/FileOrderRepositoryTest.cpp` |

---

## 7. 완료 기준

- [ ] RED: 5개 테스트 컴파일 성공 후 4개 이상 실패
- [ ] GREEN: 전체 테스트 PASSED (기존 48개 + 신규 5개 = 53개)
- [ ] 커밋①: GREEN 상태 feat 커밋
- [ ] REVIEW: 코드 리뷰 + 사용자 승인
- [ ] 커밋②: REFACTOR 반영 후 최종 커밋
