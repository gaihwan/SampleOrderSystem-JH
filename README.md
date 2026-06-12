# S-Semi 시료 생산주문 관리 시스템

> **SampleOrderSystem** — 반도체 시료(Sample)의 생산 주문을 등록·확정·생산·출하까지 전 과정을 관리하는 C++ 콘솔 애플리케이션

---

## 목차

1. [프로젝트 개요](#1-프로젝트-개요)
2. [시스템 요구사항](#2-시스템-요구사항)
3. [빌드 방법](#3-빌드-방법)
4. [실행 방법](#4-실행-방법)
   - [Release Mode — CLI 앱 실행](#41-release-mode--cli-앱-실행)
   - [Debug Mode — 테스트 실행](#42-debug-mode--테스트-실행)
5. [CLI 사용 방법](#5-cli-사용-방법)
6. [주문 상태 라이프사이클](#6-주문-상태-라이프사이클)
7. [프로젝트 구조](#7-프로젝트-구조)
8. [아키텍처](#8-아키텍처)
9. [TDD 개발 이력](#9-tdd-개발-이력)

---

## 1. 프로젝트 개요

**S-Semi**는 반도체 시료 전문 제조사로, Fabless 업체 및 연구기관에 소량의 반도체 시료를 공급합니다.

기존에는 주문 상태 추적, 납기 예측, 부서 간 정보 공유가 체계적으로 이루어지지 않아 커뮤니케이션 오류가 빈번하게 발생했습니다. 이 시스템은 주문의 전체 라이프사이클을 단일 콘솔 앱에서 관리하고, 수율 기반 배치 수와 납기를 자동으로 계산합니다.

| 항목 | 내용 |
|------|------|
| **언어** | C++17 |
| **빌드** | Visual Studio 2022 (MSBuild) |
| **테스트** | GoogleTest / GoogleMock |
| **아키텍처** | 콘솔 MVC |
| **데이터 저장** | 파일 기반 JSON 영속성 (`orders.jsonl`) |
| **플랫폼** | Windows 11 |
| **개발 방법론** | TDD (Test-Driven Development) |

---

## 2. 시스템 요구사항

- Windows 11 (x64)
- Visual Studio 2022 (Community 이상)
  - **C++ 데스크톱 개발** 워크로드 필수
- NuGet 패키지: `gmock 1.11.0` (솔루션 열기 시 자동 복원)

---

## 3. 빌드 방법

### Visual Studio IDE

1. `SampleOrderSystem-JH.sln` 파일을 Visual Studio 2022로 열기
2. NuGet 패키지 복원 (자동 또는 우클릭 → **패키지 복원**)
3. 구성 선택:
   - **Debug | x64** → 테스트 실행파일 빌드
   - **Release | x64** → CLI 앱 실행파일 빌드
4. `Ctrl+Shift+B` 로 빌드

### MSBuild (명령행)

```powershell
# Developer PowerShell for VS 2022 에서 실행
# 또는 MSBuild 전체 경로 사용

# Debug 빌드 (테스트)
& "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" `
  SampleOrderSystem-JH.sln /p:Configuration=Debug /p:Platform=x64

# Release 빌드 (CLI 앱)
& "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" `
  SampleOrderSystem-JH.sln /p:Configuration=Release /p:Platform=x64
```

---

## 4. 실행 방법

### 4.1 Release Mode — CLI 앱 실행

Release 빌드가 완료되면 **CLI 애플리케이션**이 생성됩니다.

```
x64\Release\SampleOrderSystem-JH.exe
```

실행하면 기본 제품 3종(SiC-100, SiC-200, GaN-100)이 자동으로 로드되며, 주문 데이터는 실행 디렉터리의 `orders.jsonl` 파일에 저장됩니다.

```
x64\Release\SampleOrderSystem-JH.exe
```

> 💡 **한글 출력**: 앱 시작 시 콘솔 코드 페이지를 UTF-8(65001)로 자동 설정합니다.  
> 한글이 깨지는 경우 `chcp 65001` 명령을 먼저 실행하거나 Windows Terminal 사용을 권장합니다.

### 4.2 Debug Mode — 테스트 실행

Debug 빌드가 완료되면 **GoogleTest 기반 테스트 러너**가 생성됩니다.

```powershell
# 전체 테스트 실행
.\x64\Debug\SampleOrderSystem-JH.exe

# 특정 테스트 스위트만 실행
.\x64\Debug\SampleOrderSystem-JH.exe --gtest_filter=OrderServiceTest.*

# 결과를 JSON으로 저장
.\x64\Debug\SampleOrderSystem-JH.exe --gtest_output=json:test_result.json

# 실패한 테스트만 재실행
.\x64\Debug\SampleOrderSystem-JH.exe --gtest_repeat=1 --gtest_filter=FAILED
```

현재 테스트 현황: **81개 테스트, 0개 실패**

---

## 5. CLI 사용 방법

앱 실행 시 아래 메인 메뉴가 표시됩니다:

```
=== S-Semi 시료 생산주문 관리 시스템 ===
  1.주문생성  2.주문목록  3.주문확정  4.주문반려
  5.주문취소  6.생산시작  7.출하       0.종료
>
```

### 메뉴 상세

#### 1. 주문 생성

등록된 제품 목록을 먼저 표시한 후 입력을 요청합니다.

```
[사용 가능한 제품 목록]
ID    제품명        배치크기    납기(일)  수율
1     SiC-100      50          7         0.9
2     SiC-200      25          14        0.85
3     GaN-100      20          21        0.8
  제품 ID 입력: 1
  수량 입력: 100
  마감일 입력 (YYYY-MM-DD): 2099-12-31
  >> 주문 생성 성공. 주문 ID: 1
```

- 필요 배치 수와 예상 수율이 자동으로 계산됩니다.
- `required_batches = ceil(수량 / (배치크기 × 수율))`
- `estimated_yield = 배치 수 × 배치크기 × 수율`

#### 2. 주문 목록

전체 주문을 표 형태로 출력합니다.

```
ID    수량      마감일          필요배치  상태
1     100       2099-12-31      3         RESERVED
2     200       2099-06-30      5         CONFIRMED
```

#### 3. 주문 확정

현재 주문 목록을 먼저 보여준 후 확정할 주문 ID를 요청합니다.

```
[현재 주문 목록]
...
  확정할 주문 ID 입력: 1
  >> 주문 확정 성공.
```

> ⚠️ CONFIRMED 상태인 주문이 이미 2개이면 추가 확정이 차단됩니다. (SPEC 3.3절)

#### 4. 주문 반려

RESERVED 또는 CONFIRMED 상태의 주문을 REJECTED로 처리합니다.

```
  반려할 주문 ID 입력: 1
  >> 주문 반려 성공.
```

#### 5. 주문 취소

임의의 비완료 주문을 REJECTED로 처리합니다.

```
  취소할 주문 ID 입력: 2
  >> 주문 취소 성공.
```

#### 6. 생산 시작

CONFIRMED 상태의 주문을 PRODUCING으로 전환합니다. FIFO 순서(먼저 확정된 주문 우선)를 따릅니다.

```
  생산 시작할 주문 ID 입력: 1
  >> 생산 시작 성공.
```

#### 7. 출하

PRODUCING 상태의 주문을 RELEASE로 전환합니다.

```
  출하할 주문 ID 입력: 1
  >> 출하 성공.
```

#### 0. 종료

앱을 종료합니다. 주문 데이터는 `orders.jsonl`에 자동 저장되어 다음 실행 시 복원됩니다.

### 전체 시나리오 예시

```
> 1                         ← 주문 생성
  제품 ID 입력: 1
  수량 입력: 100
  마감일 입력 (YYYY-MM-DD): 2099-12-31
  >> 주문 생성 성공. 주문 ID: 1

> 3                         ← 주문 확정
  확정할 주문 ID 입력: 1
  >> 주문 확정 성공.

> 6                         ← 생산 시작
  생산 시작할 주문 ID 입력: 1
  >> 생산 시작 성공.

> 7                         ← 출하
  출하할 주문 ID 입력: 1
  >> 출하 성공.

> 2                         ← 목록 확인
ID    수량      마감일          필요배치  상태
1     100       2099-12-31      3         RELEASE
```

---

## 6. 주문 상태 라이프사이클

```
  [주문 등록]
       │
       ▼
   RESERVED ─────────────────────────► REJECTED
       │                                 (반려/취소)
       │ 확정 (메뉴 3)
       ▼
   CONFIRMED ────────────────────────► REJECTED
       │                                 (반려/취소)
       │ 생산 시작 (메뉴 6) — FIFO 순서
       ▼
   PRODUCING ───────────────────────► REJECTED
       │                                 (취소)
       │ 출하 (메뉴 7)
       ▼
    RELEASE
```

| 상태 | 설명 | 허용 전이 |
|------|------|-----------|
| `RESERVED` | 주문 접수 완료, 확정 대기 | → CONFIRMED, REJECTED |
| `CONFIRMED` | 생산 확정, 생산 시작 대기 | → PRODUCING, REJECTED |
| `PRODUCING` | 생산 진행 중 | → RELEASE, REJECTED |
| `RELEASE` | 출하 완료 | (최종 상태) |
| `REJECTED` | 거절/취소 완료 | (최종 상태) |

---

## 7. 프로젝트 구조

```
SampleOrderSystem-JH/
├── SampleOrderSystem-JH.sln          ← Visual Studio 솔루션
├── README.md                          ← 이 파일
├── PLAN.md                            ← TDD 개발 계획서
├── CLAUDE.md                          ← Claude Code 프로젝트 규칙
│
├── doc/
│   ├── PRD.md                         ← 요구사항 정의서
│   ├── SPEC.md                        ← 기능 명세서
│   ├── COMMIT_CONVENTION.md          ← 커밋 메시지 규칙
│   └── plans/
│       ├── RED_PLAN_FS-04.md ~ RED_PLAN_FS-13.md  ← 각 Slice 계획
│       └── ...
│
├── SampleOrderSystem-JH/
│   ├── include/
│   │   ├── models/
│   │   │   ├── Order.h                ← 주문 도메인 모델
│   │   │   └── Product.h             ← 제품 도메인 모델
│   │   ├── utils/
│   │   │   ├── BatchCalculator.h     ← 배치 수 / 수율 계산
│   │   │   ├── OrderValidator.h      ← 주문 입력 유효성 검사
│   │   │   └── DummyDataGenerator.h  ← 데모/테스트용 초기 데이터
│   │   ├── repositories/
│   │   │   ├── IOrderRepository.h    ← 주문 저장소 인터페이스
│   │   │   ├── IProductRepository.h  ← 제품 저장소 인터페이스
│   │   │   ├── InMemoryOrderRepository.h
│   │   │   ├── InMemoryProductRepository.h
│   │   │   └── FileOrderRepository.h ← JSON 파일 영속성
│   │   ├── services/
│   │   │   ├── OrderService.h        ← 주문 생성/상태 전이 비즈니스 로직
│   │   │   └── ProductionService.h   ← 생산 시작/출하 비즈니스 로직
│   │   ├── controllers/
│   │   │   └── OrderController.h     ← 콘솔 입력 파싱 및 서비스 호출
│   │   ├── views/
│   │   │   ├── OrderView.h           ← 주문 목록 출력
│   │   │   └── ProductionView.h      ← 생산 현황 출력
│   │   └── app/
│   │       └── AppSession.h          ← CLI 메뉴 루프 (AppSession)
│   │
│   ├── src/
│   │   ├── views/
│   │   │   ├── OrderView.cpp
│   │   │   └── ProductionView.cpp
│   │   └── app_main.cpp              ← Release 전용 진입점 (main)
│   │
│   ├── tests/                        ← GoogleTest 테스트 파일 (Debug 전용)
│   │   ├── BatchCalculatorTest.cpp
│   │   ├── DomainModelTest.cpp
│   │   ├── OrderValidatorTest.cpp
│   │   ├── InMemoryOrderRepositoryTest.cpp
│   │   ├── InMemoryProductRepositoryTest.cpp
│   │   ├── FileOrderRepositoryTest.cpp
│   │   ├── OrderServiceCreateTest.cpp
│   │   ├── OrderServiceTransitionTest.cpp
│   │   ├── ProductionServiceTest.cpp
│   │   ├── OrderControllerTest.cpp
│   │   ├── OrderViewTest.cpp
│   │   ├── ProductionViewTest.cpp
│   │   ├── IntegrationTest.cpp
│   │   ├── DummyDataGeneratorTest.cpp
│   │   └── AppIntegrationTest.cpp
│   │
│   └── main.cpp                      ← Debug 전용 GoogleTest 진입점
│
└── packages/
    └── gmock.1.11.0/                 ← NuGet 패키지
```

> **빌드 분리 구조**
> - **Debug | x64**: `main.cpp` + `tests/*.cpp` 컴파일 → GoogleTest 러너
> - **Release | x64**: `src/app_main.cpp` 컴파일 → CLI 애플리케이션

---

## 8. 아키텍처

```
┌─────────────────────────────────────┐
│           AppSession                │
│  (메뉴 루프 / 컨텍스트 표시)          │
└────────┬──────────┬─────────────────┘
         │          │
         ▼          ▼
  OrderController  OrderView / ProductionView
  (입력 파싱)       (출력 렌더링)
         │
    ┌────┴────┐
    ▼         ▼
 OrderService  ProductionService
 (주문 비즈니스) (생산 비즈니스)
    │
    ▼
 IOrderRepository / IProductRepository
 ┌──────────────┐  ┌──────────────────┐
 │  InMemory    │  │  FileOrderRepo   │
 │  (테스트용)   │  │  (orders.jsonl)  │
 └──────────────┘  └──────────────────┘
```

### 핵심 비즈니스 규칙

| 규칙 | 공식 |
|------|------|
| 필요 배치 수 | `ceil(주문수량 / (배치크기 × 수율))` |
| 예상 생산량 | `배치 수 × 배치크기 × 수율` |
| 생산 순서 | FIFO (먼저 CONFIRMED된 주문이 먼저 생산) |
| 동시 확정 제한 | CONFIRMED 상태 주문 최대 2개 |

---

## 9. TDD 개발 이력

이 프로젝트는 **RED → GREEN → REFACTOR** 사이클을 엄격하게 준수하여 개발되었습니다.

### 개발 원칙

- 실패하는 테스트 없이 프로덕션 코드를 작성하지 않는다
- 테스트를 통과시킬 최소한의 코드만 작성한다 (YAGNI)
- 각 Feature Slice마다 코드 리뷰 후 사용자 승인을 받는다

### Feature Slice 이력

| Slice | 레이어 | 핵심 구현 | 테스트 수 |
|-------|--------|----------|-----------|
| **FS-01** BatchCalculator | Utils | 배치 수 계산 (`ceil`), 예상 수율 | 5개 |
| **FS-02** OrderValidator | Utils | 수량/납기일/제품ID 유효성 검사 | 6개 |
| **FS-03** Domain Model | Model | Order / Product 구조체 정의 | 4개 |
| **FS-04** InMemoryOrderRepository | Repository | CRUD, FindByStatus, 자동 ID | 7개 |
| **FS-05** InMemoryProductRepository | Repository | CRUD, FindById, FindAll | 4개 |
| **FS-06** OrderService (생성) | Service | CreateOrder, 배치 자동 계산 | 6개 |
| **FS-07** OrderService (전이) | Service | Confirm/Reject/Cancel 상태 전이 | 7개 |
| **FS-08** ProductionService | Service | StartProduction (FIFO), Release | 6개 |
| **FS-09** FileOrderRepository | Repository | JSON 직렬화/역직렬화, 재시작 복원 | 5개 |
| **FS-10** OrderController | Controller | 콘솔 입력 파싱, 메뉴 처리 | 8개 |
| **FS-11** OrderView / ProductionView | View | 주문 목록 출력 포맷 | 5개 |
| **FS-12** 통합 테스트 | Integration | 전체 라이프사이클, CONFIRMED 2개 제한 | 6개 |
| **FS-13** Release 앱 + DummyData | App/Utils | AppSession, DummyDataGenerator, Release 빌드 수정 | 9개 |

**총 81개 테스트 / 0개 실패**

### TDD 사이클 커밋 패턴

각 Slice는 아래 3단계 커밋으로 구성됩니다:

```
docs(plans): RED_PLAN_FS-xx 계획 수립        ← PLAN
test(xxx):   FS-xx RED — 실패 테스트 확인    ← RED
feat(xxx):   FS-xx GREEN — 최소 구현        ← GREEN  ← 사용자 코드 리뷰
refactor(xxx): FS-xx REFACTOR — 코드 정리   ← REFACTOR
```

### 주요 기술적 결정

| 결정 | 이유 |
|------|------|
| Header-only 구현 (Service/Repository) | 단일 프로젝트 구조에서 링크 복잡도 제거 |
| Debug=테스트 / Release=앱 빌드 분리 | 테스트 파일의 Release 포함 시 GTest 링크 오류 방지 |
| `u8""` 리터럴 + `/utf-8` 컴파일 플래그 | Windows MSVC에서 한글 소스 코드 안정적 처리 |
| `SetConsoleCP(CP_UTF8)` | Release 실행 시 Windows 콘솔 한글 출력 정상화 |
| FileOrderRepository (JSONL) | 재시작 후 데이터 복원 지원, 행 단위 파싱 용이 |
| FIFO 생산 순서 | 먼저 확정된 주문이 먼저 생산되도록 공정성 보장 |

---

## 라이선스

이 프로젝트는 학습 및 데모 목적으로 작성되었습니다.
