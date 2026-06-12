# RED_PLAN_FS-12 — 통합 테스트

> 작성일: 2026-06-12
> 브랜치: feat/fs-12-integration-test
> 연계: [PLAN.md §3 Phase 6 — FS-12](../../PLAN.md)

## 1. Slice 개요

| 항목 | 내용 |
|------|------|
| **Slice ID** | FS-12 |
| **레이어** | Integration |
| **핵심 책임** | 전체 라이프사이클 시나리오 검증 |
| **의존 Slice** | FS-01~FS-11 (모두) |

## 2. 테스트 케이스 목록

| # | 테스트 이름 | 검증 시나리오 |
|---|------------|-------------|
| TC-01 | `FullLifecycle_ReservedToRelease` | 주문 생성 → 확정 → 생산 → 출하 전체 흐름 |
| TC-02 | `FullLifecycle_ReservedToRejected` | 주문 생성 → 거절 흐름 |
| TC-03 | `BatchCalculation_ReflectedInCreatedOrder` | 배치 수/예상 수율이 주문에 반영됨 |
| TC-04 | `FileRepository_RestoresDataAfterRestart` | FileOrderRepository 재로드 후 데이터 복원 |
| TC-05 | `FifoProduction_ConfirmedFirstIsProducedFirst` | CONFIRMED 먼저 된 주문이 StartProduction 가능 |
| TC-06 | `ConfirmOrder_BlockedWhenTwoAlreadyConfirmed` | CONFIRMED 2개 이상이면 추가 확정 불가 (SPEC 3.3) |

## 3. 설계 결정

- TC-01~TC-05: 기존 InMemory 저장소 + Service 조합으로 테스트
- TC-06: OrderService.ConfirmOrder에 신규 비즈니스 로직 추가 필요
  - `FindByStatus(CONFIRMED).size() >= 2` 이면 실패 반환
  - 에러 메시지: "확정 대기 중인 주문이 너무 많습니다"
- TC-04: FileOrderRepository + 재생성 패턴 사용, TearDown에서 임시 파일 삭제

## 4. stub 전략

TC-01~TC-05는 기존 구현으로 바로 통과할 것이 예상된다.
TC-06만 OrderService.ConfirmOrder에 새 로직이 없으므로 실패한다.
→ RED 게이트: 최소 1개(TC-06) 실패로 조건 충족.
