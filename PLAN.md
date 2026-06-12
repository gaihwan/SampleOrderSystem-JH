# PLAN.md — SampleOrderSystem TDD 개발 계획

> 최종 수정: 2026-06-12  
> 개발 방법론: TDD (RED → GREEN → REFACTOR)  
> 빌드: CMake + Visual Studio 2022 / 테스트: GoogleTest

---

## 1. 개발 원칙

- **TDD 절대 규칙**: 실패하는 테스트 없이 프로덕션 코드를 작성하지 않는다
- **최소 구현**: 테스트를 통과시킬 최소한의 코드만 작성한다 (YAGNI)
- **레이어 순서**: 의존성이 없는 안쪽 레이어부터 바깥쪽으로 진행한다
  ```
  Utils → Domain Model → Repository → Service → Controller → View
  ```
- **커밋 단위**: 각 Feature Slice의 REFACTOR 완료 후 커밋한다

---

## 2. Feature Slice 목록

| # | Slice | 레이어 | 핵심 테스트 대상 | 의존성 |
|---|-------|--------|-----------------|--------|
| FS-01 | BatchCalculator | Utils | 배치 수 계산 공식, 올림 처리 | 없음 |
| FS-02 | OrderValidator | Utils | 입력 유효성 검사 | 없음 |
| FS-03 | Domain Model (Order / Product) | Model | 구조체 생성, 필드 접근 | 없음 |
| FS-04 | InMemoryOrderRepository | Repository | CRUD, FindByStatus | FS-03 |
| FS-05 | InMemoryProductRepository | Repository | CRUD, FindById | FS-03 |
| FS-06 | OrderService — 주문 생성 | Service | createOrder, 배치 자동 계산 | FS-01, FS-02, FS-04, FS-05 |
| FS-07 | OrderService — 상태 전이 | Service | confirm, reject, cancel | FS-06 |
| FS-08 | ProductionService | Service | startProduction(FIFO), release | FS-07 |
| FS-09 | FileOrderRepository | Repository | JSON 직렬화/역직렬화, 재시작 복원 | FS-04 |
| FS-10 | OrderController | Controller | 콘솔 입력 파싱, 메뉴 흐름 | FS-07, FS-08 |
| FS-11 | OrderView / ProductionView | View | 주문 목록 출력 형식, 상태 표시 | FS-03 |
| FS-12 | 통합 테스트 | Integration | 전체 라이프사이클 시나리오 | 모두 |

---

## 3. Phase별 개발 계획

### Phase 1 — 비즈니스 핵심 로직 (의존성 없는 레이어)

**목표**: 도메인 계산·검증 로직을 먼저 테스트로 고정한다.

---

#### FS-01: BatchCalculator

**담당**: RED 전문가 → GREEN 전문가 → 리뷰/REFACTOR/빌드/Git 전문가

| 단계 | 작업 내용 |
|------|---------|
| RED | `BatchCalculatorTest.cpp` 작성 |
| GREEN | `BatchCalculator.h / .cpp` 구현 |
| REFACTOR | 코드 리뷰 → 사용자 승인 → 정리 → 커밋 |

**테스트 목록**

```
BatchCalculatorTest_ReturnsCorrectBatchCount_WhenQuantityIsExact
BatchCalculatorTest_CeilsBatchCount_WhenQuantityHasRemainder
BatchCalculatorTest_ReturnsOneBatch_WhenQuantityIsOne
BatchCalculatorTest_ReturnsCorrectEstimatedYield
BatchCalculatorTest_HandlesEdgeCase_WhenYieldRateIsOne
```

**핵심 공식**
```
required_batches  = ceil(quantity / (batch_size × yield_rate))
estimated_yield   = required_batches × batch_size × yield_rate
```

---

#### FS-02: OrderValidator

**담당**: RED 전문가 → GREEN 전문가 → 리뷰/REFACTOR/빌드/Git 전문가

| 단계 | 작업 내용 |
|------|---------|
| RED | `OrderValidatorTest.cpp` 작성 |
| GREEN | `OrderValidator.h / .cpp` 구현 |
| REFACTOR | 코드 리뷰 → 사용자 승인 → 정리 → 커밋 |

**테스트 목록**

```
OrderValidatorTest_Valid_WhenAllFieldsCorrect
OrderValidatorTest_Invalid_WhenQuantityIsZero
OrderValidatorTest_Invalid_WhenQuantityIsNegative
OrderValidatorTest_Invalid_WhenProductIdNotFound
OrderValidatorTest_Invalid_WhenDeadlineFormatWrong      // "2025/01/01", "20250101"
OrderValidatorTest_Valid_WhenDeadlineIsToday
```

---

#### FS-03: Domain Model (Order / Product)

**담당**: RED 전문가 → GREEN 전문가 → 리뷰/REFACTOR/빌드/Git 전문가

> 구조체 정의가 목적이므로 테스트는 생성·접근·상태 열거값 수준으로 최소화한다.

**테스트 목록**

```
OrderModelTest_DefaultStatus_IsReserved
OrderModelTest_CanAccessAllFields
OrderModelTest_StatusEnum_HasAllExpectedValues   // RESERVED/CONFIRMED/PRODUCING/RELEASE/REJECTED
ProductModelTest_DefaultYieldRate_IsNinetyPercent
```

---

### Phase 2 — Repository 레이어

**목표**: InMemory 저장소로 비즈니스 로직 테스트 기반을 마련한다.  
File 기반 저장소(FS-09)는 Service/Controller가 안정된 Phase 4에서 진행한다.

---

#### FS-04: InMemoryOrderRepository

**테스트 목록**

```
InMemoryOrderRepositoryTest_Save_AssignsAutoIncrementId
InMemoryOrderRepositoryTest_FindById_ReturnsOrder_WhenExists
InMemoryOrderRepositoryTest_FindById_ReturnsNullopt_WhenNotFound
InMemoryOrderRepositoryTest_FindAll_ReturnsAllOrders
InMemoryOrderRepositoryTest_FindByStatus_FiltersCorrectly
InMemoryOrderRepositoryTest_Update_ChangesOrderStatus
InMemoryOrderRepositoryTest_FindByProductId_ReturnsMatchingOrders
```

---

#### FS-05: InMemoryProductRepository

**테스트 목록**

```
InMemoryProductRepositoryTest_FindById_ReturnsProduct_WhenExists
InMemoryProductRepositoryTest_FindById_ReturnsNullopt_WhenNotFound
InMemoryProductRepositoryTest_FindAll_ReturnsAllProducts
InMemoryProductRepositoryTest_Save_StoresProduct
```

---

### Phase 3 — Service 레이어

**목표**: 비즈니스 규칙을 Service 단위로 검증한다.

---

#### FS-06: OrderService — 주문 생성

**테스트 목록**

```
OrderServiceTest_CreateOrder_ReturnsOrderWithReservedStatus
OrderServiceTest_CreateOrder_CalculatesRequiredBatchesAutomatically
OrderServiceTest_CreateOrder_CalculatesEstimatedYieldAutomatically
OrderServiceTest_CreateOrder_Fails_WhenProductNotFound
OrderServiceTest_CreateOrder_Fails_WhenQuantityIsZero
OrderServiceTest_CreateOrder_Fails_WhenDeadlineFormatInvalid
```

---

#### FS-07: OrderService — 상태 전이

**테스트 목록**

```
OrderServiceTest_ConfirmOrder_ChangesStatus_ToConfirmed
OrderServiceTest_ConfirmOrder_Fails_WhenStatusIsNotReserved
OrderServiceTest_RejectOrder_ChangesStatus_ToRejected_FromReserved
OrderServiceTest_RejectOrder_ChangesStatus_ToRejected_FromConfirmed
OrderServiceTest_CancelOrder_ChangesStatus_ToRejected_FromAnyNonRejectedStatus
OrderServiceTest_CancelOrder_Fails_WhenAlreadyRejected
OrderServiceTest_CancelOrder_Fails_WhenOrderNotFound
```

---

#### FS-08: ProductionService

**테스트 목록**

```
ProductionServiceTest_StartProduction_ChangesStatus_ToProducing
ProductionServiceTest_StartProduction_Fails_WhenStatusIsNotConfirmed
ProductionServiceTest_StartProduction_FollowsFIFO_Order
ProductionServiceTest_Release_ChangesStatus_ToRelease
ProductionServiceTest_Release_Fails_WhenStatusIsNotProducing
ProductionServiceTest_GetProductionStatus_ReturnsConfirmedAndProducingOrders
```

---

### Phase 4 — File Repository (영속성)

---

#### FS-09: FileOrderRepository

**테스트 목록**

```
FileOrderRepositoryTest_Save_WritesJsonFile
FileOrderRepositoryTest_FindAll_ReadsFromJsonFile
FileOrderRepositoryTest_PersistsAcrossReload      // 저장 후 재로드 시 데이터 동일
FileOrderRepositoryTest_Update_OverwritesExistingEntry
FileOrderRepositoryTest_HandlesEmptyFile_GracefullyReturnsEmpty
```

---

### Phase 5 — Controller / View 레이어

---

#### FS-10: OrderController

> 콘솔 입·출력은 의존성 주입으로 추상화하여 테스트한다.

**테스트 목록**

```
OrderControllerTest_ParsesMenuInput_Correctly
OrderControllerTest_CreateOrder_CallsServiceWithParsedArgs
OrderControllerTest_ConfirmOrder_CallsServiceWithOrderId
OrderControllerTest_RejectOrder_CallsServiceWithOrderId
OrderControllerTest_CancelOrder_CallsServiceWithOrderId
OrderControllerTest_InvalidInput_ShowsErrorMessage
```

---

#### FS-11: OrderView / ProductionView

> 출력 포맷을 문자열로 캡처하여 검증한다.

**테스트 목록**

```
OrderViewTest_RenderOrderList_ShowsAllColumns         // ID/시료명/수량/납기일/상태/배치수
OrderViewTest_RenderOrderList_ShowsDeadlineAndBatch_WhenConfirmed
OrderViewTest_RenderOrderList_FiltersRejectedOrders_WhenRequested
ProductionViewTest_RenderProductionStatus_ShowsBatchProgress
ProductionViewTest_RenderProductionStatus_ShowsEstimatedCompletionDate
```

---

### Phase 6 — 통합 테스트

---

#### FS-12: 전체 라이프사이클 시나리오

**테스트 목록**

```
IntegrationTest_FullLifecycle_ReservedToRelease
IntegrationTest_FullLifecycle_ReservedToRejected
IntegrationTest_BatchCalculation_ReflectedInCreatedOrder
IntegrationTest_FileRepository_RestoresDataAfterRestart
IntegrationTest_FifoProduction_ConfirmedFirstIsProducedFirst
IntegrationTest_ConfirmOrder_BlockedWhenTwoAlreadyConfirmed
```

---

## 4. 파일 생성 순서 (빌드 의존성 기준)

```
1단계 (헤더만):
  include/models/Order.h
  include/models/Product.h
  include/utils/BatchCalculator.h
  include/utils/OrderValidator.h

2단계 (인터페이스):
  include/repositories/IOrderRepository.h
  include/repositories/IProductRepository.h

3단계 (구현체):
  include/repositories/InMemoryOrderRepository.h
  include/repositories/InMemoryProductRepository.h
  include/repositories/FileOrderRepository.h
  include/services/OrderService.h
  include/services/ProductionService.h
  include/controllers/OrderController.h
  include/controllers/ProductionController.h
  include/views/OrderListView.h
  include/views/OrderDetailView.h
  include/views/MenuView.h

4단계 (테스트 헬퍼):
  tests/helpers/ProductFactory.h
  tests/helpers/OrderFactory.h

5단계 (메인):
  src/main.cpp
  CMakeLists.txt
```

---

## 5. CMakeLists.txt 구조 (예정)

```cmake
cmake_minimum_required(VERSION 3.20)
project(SampleOrderSystem)

set(CMAKE_CXX_STANDARD 17)

# GoogleTest
include(FetchContent)
FetchContent_Declare(googletest ...)
FetchContent_MakeAvailable(googletest)

# 라이브러리 타겟
add_library(SampleOrderLib ...)

# 실행 파일
add_executable(SampleOrderSystem src/main.cpp)
target_link_libraries(SampleOrderSystem SampleOrderLib)

# 테스트
add_executable(SampleOrderSystem_Tests
    tests/BatchCalculatorTest.cpp
    tests/OrderValidatorTest.cpp
    tests/OrderServiceTest.cpp
    tests/OrderRepositoryTest.cpp
    tests/ProductionServiceTest.cpp
    tests/OrderControllerTest.cpp
    tests/OrderViewTest.cpp
)
target_link_libraries(SampleOrderSystem_Tests SampleOrderLib GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(SampleOrderSystem_Tests)
```

---

## 6. 진행 상태 추적

| Slice | RED | GREEN | REVIEW | REFACTOR | 커밋 |
|-------|:---:|:-----:|:------:|:--------:|:----:|
| FS-01 BatchCalculator | ✅ | ✅ | ✅ | ✅ | ✅ |
| FS-02 OrderValidator | ✅ | ✅ | ✅ | ✅ | ✅ |
| FS-03 Domain Model | ⬜ | ⬜ | ⬜ | ⬜ | ⬜ |
| FS-04 InMemoryOrderRepo | ⬜ | ⬜ | ⬜ | ⬜ | ⬜ |
| FS-05 InMemoryProductRepo | ⬜ | ⬜ | ⬜ | ⬜ | ⬜ |
| FS-06 OrderService 생성 | ⬜ | ⬜ | ⬜ | ⬜ | ⬜ |
| FS-07 OrderService 전이 | ⬜ | ⬜ | ⬜ | ⬜ | ⬜ |
| FS-08 ProductionService | ⬜ | ⬜ | ⬜ | ⬜ | ⬜ |
| FS-09 FileOrderRepository | ⬜ | ⬜ | ⬜ | ⬜ | ⬜ |
| FS-10 OrderController | ⬜ | ⬜ | ⬜ | ⬜ | ⬜ |
| FS-11 OrderView | ⬜ | ⬜ | ⬜ | ⬜ | ⬜ |
| FS-12 통합 테스트 | ⬜ | ⬜ | ⬜ | ⬜ | ⬜ |

> ⬜ 미시작 / 🔴 진행중(RED) / 🟢 진행중(GREEN) / 🔵 리뷰중 / ✅ 완료

---

## 7. 품질 게이트 체크리스트 (Slice 완료 기준)

```
RED 완료
  [ ] 테스트가 컴파일 오류 없이 실행됨
  [ ] 테스트가 실패함 (구현 부재가 원인)
  [ ] 테스트 이름이 SuiteName_BehaviorDescription 패턴 준수
  [ ] 하나의 테스트가 하나의 동작만 검증

GREEN 완료
  [ ] 새 테스트 전부 통과
  [ ] 기존 테스트 전부 통과
  [ ] YAGNI — 테스트 이상의 구현 없음

REVIEW 완료 (사용자 승인 필수)
  [ ] 코드 리뷰 수행
  [ ] 리뷰 결과 보고
  [ ] 사용자 명시적 승인

REFACTOR / 커밋 완료
  [ ] cmake --build 성공
  [ ] ctest --output-on-failure 전체 그린
  [ ] 컴파일러 경고 없음
  [ ] 커밋 메시지 doc/COMMIT_CONVENTION.md 준수
```

---

## 8. 참조 문서

| 문서 | 경로 |
|------|------|
| 요구사항 정의서 | `doc/PRD.md` |
| 기능 명세서 | `doc/SPEC.md` |
| 커밋 컨벤션 | `doc/COMMIT_CONVENTION.md` |
| TDD 스킬 | `.claude/skills/test-driven-development/SKILL.md` |
| Agent 팀 | `.claude/agents/README.md` |
