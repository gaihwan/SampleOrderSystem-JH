# CLAUDE.md — SampleOrderSystem

## 프로젝트 개요

**S-Semi 반도체 시료 생산주문 관리 시스템**  
반도체 시료(Sample)의 생산 주문을 RESERVED → CONFIRMED → PRODUCING → RELEASE 라이프사이클로 관리하는 C++ 콘솔 애플리케이션.

- 상세 요구사항: `doc/PRD.md`
- 기능 명세: `doc/SPEC.md`

---

## 기술 스택

| 항목 | 내용 |
|------|------|
| 언어 | C++17 |
| 빌드 | CMake + Visual Studio 2022 |
| 테스트 | GoogleTest / GoogleMock |
| 아키텍처 | 콘솔 MVC (ConsoleMVC-JH 패턴 참조) |
| 데이터 저장 | 파일 기반 JSON (DataPersistence-JH 패턴 참조) |
| 플랫폼 | Windows 11 |

---

## 개발 방법론: TDD (Test-Driven Development)

**모든 기능 개발과 버그 수정은 TDD로 진행한다.**  
구현 코드 작성 전 반드시 실패하는 테스트를 먼저 작성한다.

### TDD 사이클

```
RED (실패 테스트 작성) → GREEN (최소 구현) → REFACTOR (정리)
```

### 절대 규칙

- 실패하는 테스트 없이 프로덕션 코드를 작성하지 않는다
- 테스트가 실패하는 것을 직접 눈으로 확인한다
- 테스트를 통과시킬 최소한의 코드만 작성한다
- 예외 없음. 사용자의 명시적 허락 없이는 TDD를 건너뛰지 않는다

> 상세 TDD 규칙: `.claude/agents/skills/SKILL.md`

---

## Agent 팀

TDD 개발은 아래 에이전트 팀이 역할을 분담하여 진행한다.

| 에이전트 파일 | 역할 |
|-------------|------|
| `.claude/agents/tdd_orchestrator.md` | 총 감독 — 전체 TDD 사이클 조율 및 품질 게이트 관리 |
| `.claude/agents/tdd_red_expert.md` | RED 전문가 — 실패 테스트 작성 |
| `.claude/agents/tdd_green_expert.md` | GREEN 전문가 — 최소 구현 |
| `.claude/agents/tdd_refactor_build_git_expert.md` | REFACTOR/빌드/Git 전문가 |
| `.claude/agents/tdd_console_mvc_expert.md` | Console MVC 설계 자문 |
| `.claude/agents/tdd_data_persistence_expert.md` | Repository/모니터링 설계 자문 |
| `.claude/agents/tdd_dummy_data_generator_expert.md` | 테스트 더미 데이터 설계 |

> Agent 팀 전체 문서: `.claude/agents/README.md`

---

## Git Commit 규칙

커밋 메시지 작성 시 반드시 `doc/COMMIT_CONVENTION.md`를 참고한다.

```
<type>(<scope>): <subject>
```

- 커밋 타입, 스코프, Subject 형식을 컨벤션에 맞게 작성한다
- 하나의 커밋에는 하나의 논리적 변경만 포함한다
- `main` / `master` 브랜치에 직접 커밋하지 않는다
- `doc/*.pdf` 파일은 커밋하지 않는다 (`.gitignore` 적용됨)

> 상세 커밋 컨벤션: `doc/COMMIT_CONVENTION.md`

---

## PoC 참조 저장소

각 도메인 전문가 에이전트는 아래 PoC 저장소를 참조하여 구현 패턴을 적용한다.

| 영역 | 저장소 |
|------|--------|
| 콘솔 MVC | https://github.com/gaihwan/ConsoleMVC-JH |
| 데이터 영속성 | https://github.com/gaihwan/DataPersistence-JH |
| 데이터 모니터링 | https://github.com/gaihwan/DataMonitoring-JH |
| 더미 데이터 생성 | https://github.com/gaihwan/DummyDataGenerator-JH |

---

## 테스트 실행

```powershell
# 빌드 (Developer PowerShell for VS)
cmake --build out\build\x64-Debug

# 전체 테스트
ctest --test-dir out\build\x64-Debug --output-on-failure

# 특정 테스트만
.\out\build\x64-Debug\tests\SampleOrderSystem_Tests.exe --gtest_filter=OrderServiceTest.*

# 실패한 테스트만 재실행
ctest --test-dir out\build\x64-Debug --rerun-failed --output-on-failure
```

---

## 프로젝트 문서 구조

```
SampleOrderSystem-JH/
├── CLAUDE.md                          ← 이 파일 (프로젝트 규칙)
├── doc/
│   ├── PRD.md                         ← 요구사항 정의서
│   ├── SPEC.md                        ← 기능 명세서
│   └── COMMIT_CONVENTION.md          ← 커밋 메시지 규칙
└── .claude/
    └── agents/
        ├── README.md                  ← Agent 팀 전체 문서
        ├── tdd_orchestrator.md
        ├── tdd_red_expert.md
        ├── tdd_green_expert.md
        ├── tdd_refactor_build_git_expert.md
        ├── tdd_console_mvc_expert.md
        ├── tdd_data_persistence_expert.md
        ├── tdd_dummy_data_generator_expert.md
        └── skills/
            └── SKILL.md               ← TDD 스킬 정의
```
