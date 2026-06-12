# Commit Convention

## 커밋 메시지 형식

```
<type>(<scope>): <subject>

[body]

[footer]
```

---

## Type (필수)

| Type | 설명 |
|------|------|
| `feat` | 새로운 기능 추가 |
| `fix` | 버그 수정 |
| `refactor` | 코드 리팩토링 (기능 변경 없음) |
| `style` | 코드 포맷팅, 세미콜론 누락 등 (로직 변경 없음) |
| `docs` | 문서 추가 / 수정 |
| `test` | 테스트 코드 추가 / 수정 |
| `chore` | 빌드 설정, 패키지 관리 등 기타 작업 |
| `perf` | 성능 개선 |
| `ci` | CI/CD 설정 변경 |
| `revert` | 이전 커밋 되돌리기 |

---

## Scope (선택)

변경된 모듈 또는 기능 범위를 괄호 안에 기재합니다.

예시: `order`, `product`, `auth`, `ui`, `db`

---

## Subject (필수)

- 변경 사항을 간결하게 한 줄로 요약 (50자 이내 권장)
- 한국어 또는 영어 사용 (프로젝트 내 통일)
- 마침표(`.`) 사용 금지
- 명령형 동사로 시작 (영어일 경우: `Add`, `Fix`, `Update` 등)

---

## Body (선택)

- Subject에서 설명이 부족할 경우 작성
- 변경 이유, 변경 내용, 이전 동작과의 비교 등을 기술
- 한 줄에 72자 이내 권장
- 빈 줄로 Subject와 구분

---

## Footer (선택)

- 관련 이슈 번호 참조: `Closes #123`, `Refs #456`
- Breaking Change가 있을 경우: `BREAKING CHANGE: <설명>`

---

## 커밋 메시지 예시

### 기능 추가
```
feat(order): 주문 생성 API 추가

고객이 상품을 선택하여 주문을 생성할 수 있는 API를 구현했습니다.
주문 번호는 UUID로 자동 생성됩니다.

Closes #12
```

### 버그 수정
```
fix(product): 재고 수량 음수 저장 오류 수정

재고 차감 시 유효성 검사가 누락되어 음수 저장이 가능했던 문제를 수정했습니다.

Closes #34
```

### 리팩토링
```
refactor(auth): 토큰 검증 로직 분리

AuthService에 집중된 토큰 검증 로직을 TokenValidator 클래스로 분리했습니다.
```

### 문서 작성
```
docs: 커밋 컨벤션 초안 작성
```

---

## 주의 사항

- 하나의 커밋에는 하나의 논리적 변경만 포함합니다.
- WIP(Work In Progress) 커밋은 PR 머지 전에 squash합니다.
- `main` / `master` 브랜치에 직접 커밋하지 않습니다.
