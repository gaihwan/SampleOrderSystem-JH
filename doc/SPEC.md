# SPEC — 반도체 시료 생산주문 관리 시스템 (SampleOrderSystem)

## 1. 시스템 구조

### 아키텍처: 콘솔 MVC

```
┌────────────────────────────────────────┐
│              View Layer                │
│  OrderListView / OrderDetailView /     │
│  ProductionView / MenuView             │
└──────────────┬─────────────────────────┘
               │
┌──────────────▼─────────────────────────┐
│           Controller Layer             │
│  OrderController / ProductionController│
└──────────────┬─────────────────────────┘
               │
┌──────────────▼─────────────────────────┐
│            Service Layer               │
│  OrderService / ProductionService      │
└──────────────┬─────────────────────────┘
               │
┌──────────────▼─────────────────────────┐
│          Repository Layer              │
│  IOrderRepository / IProductRepository │
│  (InMemory / File 구현체)               │
└────────────────────────────────────────┘
```

---

## 2. 도메인 모델

### 2.1 Order (주문)

| 필드 | 타입 | 설명 |
|------|------|------|
| `id` | `int` | 주문 고유 ID (자동 생성) |
| `product_id` | `int` | 시료 ID |
| `quantity` | `int` | 주문 수량 |
| `deadline` | `std::string` | 납기일 (YYYY-MM-DD) |
| `status` | `OrderStatus` | 주문 상태 |
| `required_batches` | `int` | 필요 배치 수 (자동 계산) |
| `estimated_yield` | `double` | 예상 생산 수량 (자동 계산) |
| `created_at` | `std::string` | 주문 생성일시 |

```cpp
enum class OrderStatus {
    RESERVED,
    CONFIRMED,
    PRODUCING,
    RELEASE,
    REJECTED
};

struct Order {
    int id;
    int product_id;
    int quantity;
    std::string deadline;
    OrderStatus status;
    int required_batches;
    double estimated_yield;
    std::string created_at;
};
```

### 2.2 Product (시료)

| 필드 | 타입 | 설명 |
|------|------|------|
| `id` | `int` | 시료 고유 ID |
| `name` | `std::string` | 시료명 |
| `batch_size` | `int` | 배치당 생산 수량 |
| `batch_days` | `int` | 배치당 생산 기간 (일) |
| `yield_rate` | `double` | 수율 (0.0 ~ 1.0) |

```cpp
struct Product {
    int id;
    std::string name;
    int batch_size;
    int batch_days;
    double yield_rate;  // 기본값: 0.9
};
```

---

## 3. 기능 명세

### 3.1 주문 목록 조회 (Order List Display)

**화면 구성**

```
=== 주문 목록 ===
ID | 시료명     | 수량 | 납기일     | 상태      | 배치수
---+------------+------+------------+-----------+------
 1 | SiC-100    |  500 | 2025-03-01 | RESERVED  |   6
 2 | GaN-200    |  200 | 2025-02-15 | CONFIRMED |   3
 3 | SiC-100    |  100 | 2025-01-30 | PRODUCING |   2
```

**조건**
- 전체 주문을 최신 등록 순으로 표시한다
- 상태별 필터링: RESERVED / CONFIRMED / PRODUCING / RELEASE / REJECTED / 전체
- CONFIRMED 상태는 납기일과 배치 수를 함께 표시한다
- 취소(REJECTED) 주문은 별도 표시 또는 필터로 숨길 수 있다

---

### 3.2 시료 주문 생성 (Order Create)

**입력 항목**

| 항목 | 타입 | 유효성 조건 |
|------|------|-----------|
| 시료 ID | `int` | 등록된 시료 ID여야 한다 |
| 주문 수량 | `int` | 1 이상의 정수 |
| 납기일 | `string` | YYYY-MM-DD 형식 |

**처리 로직**

1. 입력값 유효성 검사
2. 수율 기반 필요 배치 수 계산
   ```
   required_batches = ceil(quantity / (batch_size × yield_rate))
   ex) 수량 100, 배치크기 20, 수율 0.9 → ceil(100 / (20 × 0.9)) = ceil(5.56) = 6
   ```
3. 예상 생산 수량 계산
   ```
   estimated_yield = required_batches × batch_size × yield_rate
   ```
4. 상태 = RESERVED 로 저장

**출력**
- 생성된 주문 ID 표시
- 계산된 배치 수, 예상 생산 수량 표시

**오류 처리**
- 존재하지 않는 시료 ID → "시료를 찾을 수 없습니다"
- 수량 0 이하 → "수량은 1 이상이어야 합니다"
- 납기일 형식 오류 → "날짜 형식이 올바르지 않습니다 (YYYY-MM-DD)"

---

### 3.3 주문 확정 / 거절 (Order Confirm / Reject)

**전제 조건**: 대상 주문의 상태가 RESERVED

**확정 (CONFIRMED)**

```
입력: 주문 ID
처리:
  1. RESERVED 상태 확인
  2. 현재 PRODUCING 중인 주문 수 확인
  3. 납기 재계산 (현재 시점 기준)
  4. 상태 → CONFIRMED
출력: RESERVED 목록에서 CONFIRMED 항목으로 이동하여 Display
```

**조건**
- 이미 2개 이상 CONFIRMED 상태인 경우 → 추가 확정 불가 (대기)
- 같은 시료의 기존 주문이 PRODUCING 중이면 → CONFIRMED 상태로 대기

**거절 (REJECTED)**

```
입력: 주문 ID
처리:
  1. RESERVED 상태 확인
  2. 상태 → REJECTED
출력: 거절 처리 완료 메시지
```

---

### 3.4 주문 취소 / 강제 거절 (Order Cancel / Force Reject)

**대상 상태**: RESERVED / CONFIRMED / PRODUCING / RELEASE

**처리 로직**

```
입력: 주문 ID
처리:
  1. 대상 주문 상태 확인
  2. 유효 상태(RESERVED/CONFIRMED/PRODUCING/RELEASE) 확인
  3. 상태 → REJECTED
출력: 거절 처리 완료 메시지
조건:
  - 재고(stock) 복구: 해당 없음 (시료 생산 주문이므로)
  - REJECTED 수량: 0으로 처리
```

**오류 처리**
- 이미 REJECTED인 주문 → "이미 거절된 주문입니다"
- 존재하지 않는 주문 ID → "주문을 찾을 수 없습니다"

---

### 3.5 생산 관리 (Production Management)

#### 3.5.1 생산 현황 조회

```
=== 생산 현황 ===
시료명    | 생산중 배치 | 완료 예정일 | 상태
----------+--------------+-------------+-----------
SiC-100   |     3/6      | 2025-02-20  | PRODUCING
GaN-200   |     0/3      | 2025-03-01  | CONFIRMED
```

- CONFIRMED / PRODUCING 주문의 배치 진행 현황 표시
- 완료 예정일 = 생성일 + (배치 수 × 배치당 생산 기간)

#### 3.5.2 생산 시작 (CONFIRMED → PRODUCING)

**전제 조건**: 대상 주문의 상태가 CONFIRMED

```
입력: 주문 ID
처리:
  1. CONFIRMED 상태 확인
  2. 상태 → PRODUCING
출력: 생산 시작 완료 메시지, 예상 완료일 표시
조건:
  - FIFO 원칙: 먼저 CONFIRMED된 주문이 먼저 PRODUCING 전환
  - 동시에 PRODUCING 가능한 주문 수 제한 없음
```

#### 3.5.3 출하 처리 (PRODUCING → RELEASE)

**전제 조건**: 대상 주문의 상태가 PRODUCING

```
입력: 주문 ID
처리:
  1. PRODUCING 상태 확인
  2. 상태 → RELEASE
출력: 출하 완료 메시지
```

---

### 3.6 배치 계산 공식

| 항목 | 공식 | 예시 |
|------|------|------|
| 필요 배치 수 | `ceil(수량 / (배치크기 × 수율))` | ceil(100 / (20 × 0.9)) = 6 |
| 예상 생산 수량 | `배치 수 × 배치크기 × 수율` | 6 × 20 × 0.9 = 108 |
| 예상 완료일 | `생성일 + (배치 수 × 배치당생산일)` | 생성일 + 6 × 3일 = +18일 |

---

## 4. 화면 구성 (콘솔 UI)

### 메인 메뉴

```
============================================
      S-Semi 시료 생산주문 관리 시스템
============================================
1. 주문 목록 조회
2. 시료 주문 생성
3. 주문 확정 / 거절
4. 주문 취소
5. 생산 관리
   5-1. 생산 현황 조회
   5-2. 생산 시작 (CONFIRMED → PRODUCING)
   5-3. 출하 처리 (PRODUCING → RELEASE)
0. 종료
============================================
선택 > 
```

---

## 5. 파일 저장 구조 (Data Persistence)

```
data/
├── orders.json      ← 전체 주문 데이터
└── products.json    ← 시료 마스터 데이터
```

### orders.json 예시

```json
[
  {
    "id": 1,
    "product_id": 1,
    "quantity": 100,
    "deadline": "2025-03-01",
    "status": "RESERVED",
    "required_batches": 6,
    "estimated_yield": 108.0,
    "created_at": "2025-01-15T09:00:00"
  }
]
```

---

## 6. 상태 전이 규칙 (State Machine)

```
RESERVED ──[확정]──► CONFIRMED ──[생산시작]──► PRODUCING ──[출하]──► RELEASE
    │                    │
    └──[거절]──► REJECTED └──[거절]──► REJECTED
    │
    └──[취소]──► REJECTED (RESERVED/CONFIRMED/PRODUCING/RELEASE 모두 가능)
```

| 현재 상태 | 허용 전이 |
|---------|---------|
| RESERVED | CONFIRMED, REJECTED |
| CONFIRMED | PRODUCING, REJECTED |
| PRODUCING | RELEASE, REJECTED |
| RELEASE | REJECTED |
| REJECTED | (없음, 최종 상태) |

---

## 7. 테스트 범위

### 단위 테스트 (GoogleTest)

| 모듈 | 테스트 항목 |
|------|-----------|
| `BatchCalculator` | 수율 기반 배치 수 계산, 올림 처리, 경계값 |
| `OrderValidator` | 유효한 주문 / 수량 0 이하 / 잘못된 날짜 형식 |
| `OrderService` | 주문 생성, 확정, 거절, 취소, 생산 시작, 출하 |
| `OrderRepository` | CRUD, FindByStatus, FindByProductId |
| `ProductionService` | FIFO 순서, CONFIRMED → PRODUCING, PRODUCING → RELEASE |
| `OrderController` | 콘솔 입력 파싱, 메뉴 흐름 |
| `OrderView` | 주문 목록 출력 형식, 상태 표시 |

### 통합 테스트

- 전체 주문 라이프사이클 (RESERVED → RELEASE) 시나리오
- 수율 계산 결과가 실제 주문에 반영되는지 검증
- 파일 저장 후 재시작 시 데이터 복원

---

## 8. 프로젝트 파일 구조

```
SampleOrderSystem-JH/
├── include/
│   ├── models/
│   │   ├── Order.h
│   │   └── Product.h
│   ├── services/
│   │   ├── OrderService.h
│   │   └── ProductionService.h
│   ├── repositories/
│   │   ├── IOrderRepository.h
│   │   ├── IProductRepository.h
│   │   ├── InMemoryOrderRepository.h
│   │   ├── FileOrderRepository.h
│   │   └── InMemoryProductRepository.h
│   ├── controllers/
│   │   ├── OrderController.h
│   │   └── ProductionController.h
│   ├── views/
│   │   ├── OrderListView.h
│   │   ├── OrderDetailView.h
│   │   └── MenuView.h
│   └── utils/
│       ├── BatchCalculator.h
│       └── OrderValidator.h
├── src/
│   ├── models/
│   ├── services/
│   ├── repositories/
│   ├── controllers/
│   ├── views/
│   └── utils/
├── tests/
│   ├── helpers/
│   │   ├── ProductFactory.h
│   │   └── OrderFactory.h
│   ├── BatchCalculatorTest.cpp
│   ├── OrderValidatorTest.cpp
│   ├── OrderServiceTest.cpp
│   ├── OrderRepositoryTest.cpp
│   ├── ProductionServiceTest.cpp
│   ├── OrderControllerTest.cpp
│   └── OrderViewTest.cpp
├── data/
│   ├── orders.json
│   └── products.json
└── CMakeLists.txt
```
