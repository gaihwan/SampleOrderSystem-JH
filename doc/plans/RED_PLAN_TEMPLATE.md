# RED_PLAN_FS-xx — [Slice 이름]

> 작성일: YYYY-MM-DD  
> 브랜치: feat/fs-xx-[slice-name]  
> 연계: [PLAN.md §3 Phase X — FS-xx](../../PLAN.md)

---

## 1. Slice 개요

| 항목 | 내용 |
|------|------|
| **Slice ID** | FS-xx |
| **레이어** | Utils / Model / Repository / Service / Controller / View |
| **핵심 책임** | 이 Slice가 담당하는 단일 책임 한 줄 요약 |
| **의존 Slice** | FS-xx, FS-xx (없으면 "없음") |

---

## 2. PLAN.md 연계

> PLAN.md Feature Slice 목록 중 해당 항목:

```
| FS-xx | [Slice명] | [레이어] | [핵심 테스트 대상] | [의존성] |
```

---

## 3. 설계 결정

### 인터페이스 설계

```cpp
// 테스트가 사용할 공개 API 스케치
// (구현 전 인터페이스를 먼저 결정한다)
```

### 설계 결정 사항

| 결정 | 이유 |
|------|------|
| (예) 헤더 전용 inline | 단순 유틸리티, .cpp 분리 불필요 |
| (예) static 메서드 | 상태 없는 순수 함수 |

### 의존성 처리

- 미구현 의존성(예: Repository)이 있을 경우 어떻게 테스트하는가?
- (예) `std::unordered_set<int>` 형태의 유효 ID 집합을 인자로 주입

---

## 4. 테스트 케이스 목록

| # | 테스트 이름 | 검증 동작 | 입력 | 기대 출력 |
|---|------------|---------|------|---------|
| 1 | `SuiteTest_BehaviorDescription` | ... | ... | ... |
| 2 | ... | ... | ... | ... |

---

## 5. stub 전략

RED 단계에서 컴파일은 되지만 테스트는 실패하도록 stub을 작성한다.

```cpp
// stub 예시 — 항상 기본값/0 반환
```

---

## 6. 파일 목록

| 역할 | 경로 |
|------|------|
| 헤더 (stub) | `SampleOrderSystem-JH/include/[layer]/[Name].h` |
| 테스트 | `SampleOrderSystem-JH/tests/[Name]Test.cpp` |

---

## 7. 완료 기준

- [ ] RED: 테스트가 컴파일 성공 후 실패
- [ ] GREEN: 전체 테스트 PASSED
- [ ] 커밋①: GREEN 상태 feat 커밋
- [ ] REVIEW: 코드 리뷰 + 사용자 승인
- [ ] 커밋②: REFACTOR 반영 후 최종 커밋
