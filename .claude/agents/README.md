# SampleOrderSystem — TDD Agent 팀

SampleOrderSystem을 TDD 방식으로 개발하기 위한 Claude Agent 팀 구성입니다.  
모든 에이전트는 `doc/COMMIT_CONVENTION.md`와 `.claude/skills/test-driven-development/SKILL.md`에 정의된 TDD 원칙을 준수합니다.

---

## 팀 구성

```
tdd_orchestrator
      │
      ├── tdd_red_expert               ← RED 단계
      ├── tdd_green_expert             ← GREEN 단계
      └── tdd_review_refactor_build_git_expert ← 리뷰 + REFACTOR + 빌드 + Git
                │
                └── (설계 자문)
                      ├── tdd_console_mvc_expert
                      ├── tdd_data_persistence_expert
                      └── tdd_dummy_data_generator_expert
```

---

## 에이전트 목록

| 파일 | 역할 | 설명 |
|------|------|------|
| `tdd_orchestrator.md` | **총 감독** | 전체 TDD 사이클 조율, 품질 게이트 관리, 에이전트 위임 |
| `tdd_red_expert.md` | **RED 전문가** | 실패하는 테스트 작성, 올바른 실패 확인 |
| `tdd_green_expert.md` | **GREEN 전문가** | 최소 구현으로 테스트 통과, YAGNI 원칙 준수 |
| `tdd_review_refactor_build_git_expert.md` | **리뷰/REFACTOR/빌드/Git 전문가** | 코드 리뷰, 사용자 승인 요청, 코드 정리, 빌드 검증, 커밋 컨벤션 적용 |
| `tdd_console_mvc_expert.md` | **Console MVC 전문가** | UI/Controller 설계 자문, 콘솔 MVC 패턴 지원 |
| `tdd_data_persistence_expert.md` | **Data Persistence 전문가** | Repository 패턴, 파일/DB 영속성, 모니터링 설계 |
| `tdd_dummy_data_generator_expert.md` | **DummyDataGenerator 전문가** | 테스트 픽스처, 더미 데이터 팩토리 설계 |

---

## TDD 사이클 워크플로우

```
[Orchestrator] 요구사항 접수 → Feature Slice 분해
        │
        ▼
[RED 전문가] 실패 테스트 작성
        │  (Console MVC / Data Persistence / DummyDataGenerator 에게 자문 가능)
        │
        ▼  품질 게이트: 올바른 이유로 실패하는가?
        │
[GREEN 전문가] 최소 구현
        │  (Console MVC / Data Persistence / DummyDataGenerator 에게 자문 가능)
        │
        ▼  품질 게이트: 전체 테스트 그린인가?
        │
[리뷰/REFACTOR/빌드/Git 전문가] 코드 리뷰 → 사용자 승인 → 정리 + 빌드 검증 + 커밋
        │
        ▼  품질 게이트: 리뷰 승인, 빌드 성공, 경고 없음, 컨벤션 준수
        │
[Orchestrator] 다음 Feature Slice
```

---

## 도메인 전문가 협업 규칙

RED/GREEN 전문가는 다음 경우 도메인 전문가에게 **설계 자문**을 요청할 수 있습니다.

| 상황 | 자문 대상 |
|------|---------|
| UI 입력/출력 형식, Controller 인터페이스 | Console MVC 전문가 |
| Repository 인터페이스, 저장 형식, 모니터링 | Data Persistence 전문가 |
| 테스트용 샘플 데이터, 픽스처 셋업 | DummyDataGenerator 전문가 |

---

## PoC 참조 저장소

도메인 전문가들은 아래 PoC 저장소 코드를 참조하여 SampleOrderSystem에 적용합니다.

| 전문가 | 저장소 |
|--------|--------|
| Console MVC 전문가 | https://github.com/gaihwan/ConsoleMVC-JH |
| Data Persistence 전문가 | https://github.com/gaihwan/DataPersistence-JH |
| Data Monitoring 전문가 | https://github.com/gaihwan/DataMonitoring-JH |
| DummyDataGenerator 전문가 | https://github.com/gaihwan/DummyDataGenerator-JH |

---

## 품질 게이트 요약

### RED 게이트 (RED 전문가 → Orchestrator)
- 테스트가 컴파일/링커 오류 없이 실행된다
- 테스트가 실패한다 (통과하면 안 됨)
- 실패 이유가 구현 부재다
- 하나의 테스트 = 하나의 동작

### GREEN 게이트 (GREEN 전문가 → REFACTOR 전문가)
- 새 테스트가 통과한다
- 기존 테스트가 모두 통과한다
- 최소 구현이다 (YAGNI)

### REFACTOR 게이트 (REFACTOR 전문가 → Orchestrator)
- `cmake --build` 성공
- `ctest --output-on-failure` 전체 그린
- 컴파일러 경고 없음
- 커밋 컨벤션 준수 (`doc/COMMIT_CONVENTION.md`)

---

## 기술 스택

- **언어**: C++17
- **빌드**: CMake + Visual Studio 2022
- **테스트**: GoogleTest / GoogleMock
- **플랫폼**: Windows 11

---

## 관련 문서

- TDD 스킬 정의: `.claude/skills/test-driven-development/SKILL.md`
- 커밋 컨벤션: `doc/COMMIT_CONVENTION.md`
- 프로젝트 CLAUDE 규칙: `CLAUDE.md`
