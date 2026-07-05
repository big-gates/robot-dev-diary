# Chapter C 학습 노트 — 컨테이너에서 C++ 디버깅: 세팅과 층별 격리 진단기

> "F5를 눌렀는데 노란 줄이 안 뜬다"에서 시작해 CodeLLDB 정착까지, 실제로 겪은 순서 그대로 정리한 노트.
> 환경: macOS(Apple Silicon) + VS Code Dev Container(Ubuntu 24.04, gdb 15.1, aarch64).

## 0. 최종 구성 (결론 먼저)

| 도구 | 역할 |
|---|---|
| clangd | 자동완성·진단 (이해 라인) |
| **CodeLLDB** (`vadimcn.vscode-lldb`) | **디버깅 (기본)** — 자체 엔진 내장, 컨테이너에서 신뢰성 높음 |
| cpptools | 예비 디버그 어댑터 (IntelliSense는 꺼둠 — clangd와 충돌) |
| devcontainer `runArgs` | `--cap-add=SYS_PTRACE`, `--security-opt seccomp=unconfined` — **컨테이너 C++ 디버깅 필수** |

- `tasks.json`: 현재 파일을 `-std=c++17 -Wall -Wextra -g -O0`로 빌드 (`${file}` → 활성 탭)
- `launch.json`: lldb 구성이 기본, cppdbg는 예비. `preLaunchTask`로 빌드 자동 연결
- **`-g`** = 기계어↔소스 매핑(디버그 심볼), **`-O0`** = 최적화 끔(줄 번호 일치). 디버그 빌드 국룰

## 1. 운전법 요약

- **F5의 과녁 = 지금 활성화된 에디터 탭** (`${file}` 패턴). 디버깅할 .cpp를 클릭하고 F5
- F9 브레이크포인트 / F5 시작·계속 / F10 한 줄씩 / F11 함수 안으로 / ⏹ 종료
- 진실의 원천은 **Run and Debug 패널**(`Cmd+Shift+D`): VARIABLES, CALL STACK, 세션 개수. 에디터 노란 줄은 반사일 뿐
- **디버그 툴바가 떠 있다 = 세션이 살아 있다.** 그 상태에서 F5 재시작 금지 — 세션이 겹쳐 쌓인다 (좀비 6개 실화)
- assert가 터지면 디버거가 그 자리에서 멈춤 → CALL STACK에서 `abort → __assert_fail → main` 층계의 main 클릭 = 터진 줄로 점프

## 2. 무해한 경고 목록 (컨테이너 환경 소음)

| 메시지 | 정체 |
|---|---|
| `GDB: Failed to set controlling terminal: Operation not permitted` | 컨테이너에서 TTY 제어권을 못 가져감. 디버깅 무관 |
| `Error disabling address space randomization: Operation not permitted` | seccomp가 ASLR 끄기를 차단(아래 3절로 해결 가능). 멈춤 자체엔 무관했음 |
| `&"..."` 포장 | gdb↔VS Code의 GDB/MI 프로토콜 로그 스트림 표기 |

판별 원칙: **경고 문구가 아니라 기능으로 판단** — 브레이크포인트에서 멈추고 변수가 보이면 정상.

## 3. 실전 사건: "멈췄는데 UI는 Running" — 층별 격리 진단기

증상: F5 → 빌드 성공 → 세션 생성 → 브레이크포인트 노란 줄 없음, CALL STACK "Running" 고정.

진단은 **아래층부터 한 층씩 검증**했다 (Chapter B의 "어느 단계의 실패인가" 프레임 그대로):

| 층 | 검증 방법 | 결과 |
|---|---|---|
| ① 바이너리+심볼 | `gdb -batch -ex 'break main' -ex run ./robot_utils` | ✓ main에서 멈춤 |
| ② 프로세스 실태 | `ps -eo pid,stat,wchan,cmd` → 디버기 상태 **`ts` + `ptrace_stop`** | ✓ 실제로는 멈춰 있음! |
| ③ 시스템 권한 | `setarch $(uname -m) -R true` → `Operation not permitted` | ✗ seccomp 차단 발견 → runArgs로 해제 |
| ④ (해제 후에도 재발) MI 프로토콜 | `printf -- '-break-insert main\n-exec-run\n' \| gdb --interpreter=mi ./bin` | ✓ `*stopped,reason="breakpoint-hit"` 정상 발생 |
| ⑤ 어댑터 | 남은 유일한 층 | ✗ **범인** — cpptools ↔ gdb 15.1(aarch64) 조합에서 멈춤 이벤트 미반영 |

해법: 어댑터 교체 — **CodeLLDB**로 전환 즉시 해결. (gdb 자체는 결백했다.)

핵심 명령 세 개는 외울 가치가 있다:
```bash
ps -eo pid,stat,wchan:20,cmd | grep <이름>   # ts+ptrace_stop = "디버거 아래 멈춤"
docker inspect <id> --format '{{.HostConfig.SecurityOpt}} {{.HostConfig.CapAdd}}'
pkill -9 gdb                                  # 좀비 세션 청소
```

## 4. 같은 날 수집한 에러 도감 (g++ 4단계 파이프라인 완주)

| 단계 | 에러 | 원인 |
|---|---|---|
| 컴파일 | `'average_range' was not declared ... did you mean 'average_ragne'?` | 함수 정의부 오타. **컴파일러의 did-you-mean은 읽으라고 있는 것** |
| 컴파일(경고) | `no return statement in function returning non-void` | return 누락 — C++은 이걸 **경고**로만 알리고 쓰레기값을 반환한다. `-Wall` 필수인 이유 |
| 링크 | main 중복 정의 | `g++ ... robot_utils robot_utils.cpp` — **`-o` 누락**으로 기존 실행 파일이 입력으로 들어감 |
| 런타임 | assert 실패 | `bool clamp_speed(...)` — double 반환값이 bool로 **암묵 변환**. 컴파일러는 침묵, 테스트가 잡음 |

교훈: 재타이핑은 사고의 근원 — ↑화살표, Ctrl+R, 그리고 결국 빌드 자동화(tasks.json → E장 CMake).
