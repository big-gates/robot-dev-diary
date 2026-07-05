# Chapter A 학습 노트 — C++/ROS 2 툴체인의 큰 그림

> Chapter A를 진행하며 나온 질문들을 AI 과외(Claude) 문답으로 정리한 노트.
> 백엔드(Java/Kotlin + Gradle + IntelliJ) 출신 개발자의 눈높이로, 실제 이 레포에서 벌어진 일들을 예시로 쓴다.

---

## 0. 마스터 멘탈 모델: 라인이 두 개다

자바 세계에서는 IntelliJ 하나가 모든 걸 한 몸에 담고 있어서 경계가 안 보인다. C++ 세계는 전부 **독립된 프로그램들**로 쪼개져 있고, 서로 완전히 다른 두 라인이 돈다:

```
[라인 1 — 생산 라인]  "실행 파일을 만든다"        colcon build 칠 때만 동작
colcon → CMake → make/ninja → g++ 컴파일러 → 실행 파일

[라인 2 — 이해 라인]  "에디터가 코드를 이해한다"    타이핑하는 내내 동작
VS Code ←(LSP)→ clangd
```

두 라인은 독립적이다 — clangd가 죽어도 빌드는 되고, 빌드 시스템 없이도 clangd는 (아래의 '다리'만 있으면) 돈다. 만나는 지점은 단 하나, `compile_commands.json`.

| C++ 세계 | 하는 일 | 자바 세계 대응 |
|---|---|---|
| g++ | 진짜 컴파일 | javac |
| CMake + colcon | 빌드 지휘 | Gradle |
| clangd | 에디터용 실시간 코드 이해 | IntelliJ 내장 분석 엔진 |
| compile_commands.json | 두 라인을 잇는 다리 | IntelliJ의 Gradle 임포트 |
| VS Code | 그림 그리는 껍데기 | IntelliJ의 화면 부분 |

---

## 1. 생산 라인 해부 (아래에서 위로)

핵심 문장: **각 층은 아래층의 고통에서 태어났다.**
g++ 직접 호출의 고통 → make. Makefile 수제작의 고통 → CMake. 패키지 수십 개 반복의 고통 → colcon.

### 1-1. g++ — 진짜 노동자

GNU의 C++ 컴파일러. 다른 모든 도구는 결국 g++를 올바른 옵션으로 호출하기 위한 비서들이다.
(우분투의 `/usr/bin/c++`는 g++를 가리키는 링크. compile_commands.json에서 확인 가능.)

4단계 파이프라인과 에러 대응표:

| 단계 | 하는 일 | 이 단계의 비명 |
|---|---|---|
| ① 전처리 | `#include`를 말 그대로 텍스트로 붙여넣음, 매크로 치환 | `'rclcpp/rclcpp.hpp' file not found` |
| ② 컴파일 | C++ → 기계어 번역, 타입 검사, 템플릿, 최적화 | 에러 메시지의 90% |
| ③ 어셈블 | `.o` 오브젝트 파일 생성 (build/에 쌓이는 것) | — |
| ④ 링크 | `.o`들 + 라이브러리(.so)를 실행 파일로 결합 | `undefined reference to ...` |

javac와의 결정적 차이: javac는 JVM용 바이트코드를 만들지만 **g++는 CPU가 직접 먹는 기계어**를 만든다. 그래서 결과물이 CPU 아키텍처(arm64/amd64)에 묶인다 — Chapter A의 amd64 이미지 함정이 정확히 이것. VM도 GC도 없이 맨몸으로 돌아서 로봇/임베디드가 C++을 쓴다.

라이벌: clang++ (LLVM 진영). 같은 표준을 구현해 호환된다.

30초 실험 — 비서 없이 주인공만 부르기:
```bash
echo '#include <iostream>
int main() { std::cout << "raw g++!\n"; }' > /tmp/hi.cpp
g++ /tmp/hi.cpp -o /tmp/hi && /tmp/hi
```

### 1-2. make와 ninja — 레시피 실행기 (같은 직업)

**Makefile은 레시피 파일이다**: "무엇을 만들려면 뭐가 필요하고 어떤 명령을 치는가"의 목록.

```make
hello_node: hello_node.o           # 타깃: 재료
	g++ hello_node.o -o hello_node   # 만드는 명령
```

make(1977년생)의 초능력은 **타임스탬프 비교** — 재료가 안 바뀐 타깃은 건너뛴다(증분 빌드).

**ninja(2010년생)는 make와 같은 자리에 꽂히는 고속 후계자.** 구글 크롬 팀이 "파일 수만 개 규모에서는 make가 변경 계산조차 느리다"며 만들었다. 사람이 읽고 쓰는 기능을 걷어내고 실행 속도에 올인했으며, build.ninja는 기계(CMake)가 생성하는 전제로 설계됐다. CMake에 `-G Ninja`를 주면 하층만 교체된다.

### 1-3. CMake — 레시피 생성기 (메타 빌드 시스템)

CMake는 빌드하지 않는다. **CMakeLists.txt(사람이 쓰는 고수준 선언)를 읽어 Makefile/build.ninja(기계용 저수준 명령 목록)를 생성**한다.

실측: 이 레포의 hello_ros는 13줄짜리 CMakeLists.txt였고, CMake가 269줄짜리 Makefile을 생성했다. 그 변환이 관계의 전부다.

```
CMakeLists.txt (13줄, 설계 의도)
      │  cmake = 번역가
      ▼
Makefile (269줄, 시공 지시서)
      │  make = 현장 반장 (바뀐 것만)
      ▼
g++ 호출들 (실제 시공)
```

존재 이유는 크로스 플랫폼: 같은 CMakeLists로 리눅스에선 Makefile, 윈도우에선 Visual Studio 프로젝트, 맥에선 Xcode 프로젝트를 생성한다.

### 1-4. colcon — 워크스페이스 오케스트레이터

이름은 **col**lective **con**struction. **ROS 2 세계의 `./gradlew build`.**

`colcon build`가 하는 일:
1. `src/` 스캔 → package.xml 있는 폴더를 패키지로 발견 (≈ settings.gradle의 모듈 등록, 단 자동)
2. `<depend>` 선언으로 의존성 그래프 → 위상 정렬로 빌드 순서 결정
3. 의존 없는 패키지끼리 병렬 빌드
4. 패키지마다 `<build_type>`을 보고 적절한 백엔드에 **위임** (colcon 자신은 컴파일을 전혀 안 함)
5. 산출물 정리:

| 폴더 | 역할 | 비고 |
|---|---|---|
| `build/` | 패키지별 작업장 (CMake 실행, .o, compile_commands.json) | 지워도 됨 (재빌드 시간만 손해) |
| `install/` | 완성품 진열대 + setup.bash | 이것만 있으면 실행 가능 |
| `log/` | 실행별 상세 로그 (`log/latest_build/<pkg>/stdout_stderr.log`) | 에러 디버깅 보물창고 |

셋 다 재생성 가능한 산출물 → `.gitignore` 대상.

자주 쓰는 명령:
```bash
colcon build                              # 전체 (변경분만)
colcon build --packages-select hello_ros  # 특정 패키지만
colcon list                               # 패키지 목록
colcon test && colcon test-result --verbose
```

⚠️ 함정: colcon은 **현재 디렉토리**를 워크스페이스로 믿는다. `src/` 안에서 실행하면 src 안에 build/가 생기는 대참사. 항상 워크스페이스 루트에서.

역사: ROS 1은 catkin을 썼고, ROS 2에서 colcon으로 세대교체. colcon은 ROS 전용이 아닌 범용 도구다.

---

## 2. 이해 라인: clangd와 다리

### 2-1. clangd — 에디터의 두뇌

VS Code 옆에 상주하는 **언어 서버**. 키 입력마다 LSP(Language Server Protocol)로 대화하며, 컴파일러(clang)의 파서 부분만 돌려 코드를 즉석 분석한다. 실행 파일은 절대 만들지 않는다. 자동완성·빨간 물결·정의 점프는 전부 clangd가 계산하고 VS Code는 그리기만 한다.

(자바도 VS Code에서는 같은 구조다 — Eclipse JDT LS라는 언어 서버가 뒤에 뜬다. IntelliJ는 이걸 내장해서 안 보였을 뿐.)

### 2-2. compile_commands.json — 유일한 다리

clangd가 코드를 이해하려면 컴파일러와 똑같은 정보(-I 인클루드 경로, -D 매크로, -std 등)가 필요한데, 그건 빌드 설정만 안다. 그래서 **생산 라인이 남긴 작업 일지를 컨닝**한다:

```
라인 1 ──빌드하며 기록──▶ build/compile_commands.json ──읽음──▶ clangd
```

- 레포 루트의 `.clangd` 파일이 일지 위치(`CompilationDatabase: ros2_ws/build/`)를 알려주는 쪽지
- CMake가 이 JSON을 만들 수 있는 이유: 레시피 작성자라서 모든 컴파일 명령을 이미 알기 때문

### 2-3. `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`

- `-D키=값` = CMake 변수 정의. colcon의 `--cmake-args`가 각 패키지의 CMake 호출에 그대로 전달
- **안 쓰면?** 빌드는 100% 동일. 대신 일지가 안 생겨 에디터만 바보가 된다
- ⚠️ 교묘한 함정: 예전 일지가 남아 있으면 당장은 멀쩡해 보이다가, **새 파일에만 빨간 물결**이 뜨는 미스터리 증상 (낡은 compile_commands.json). 해법: 옵션 켜고 재빌드 + `clangd: Restart language server`
- 영구 박제: CMake 3.17+는 같은 이름의 환경변수를 기본값으로 읽는다. Dockerfile에 `ENV CMAKE_EXPORT_COMPILE_COMMANDS=1` 한 줄 → 이후 `colcon build`만 쳐도 됨. 환경의 진실은 Dockerfile에("새 맥북 30분 재현" 원칙)
- 기본값이 OFF인 이유: CI/배포 빌드엔 쓸모없는 부산물이라 opt-in이 CMake 철학

---

## 3. ROS 2 코어 개념

### 3-1. Distro 체계 — Humble vs Jazzy

ROS 2는 우분투처럼 매년 5월, 알파벳 순서로 배포판이 나온다. **백엔드의 "Java 11 vs 17, Spring Boot 2 vs 3"과 같은 관계** — 개념·API의 99%는 동일.

| Distro | 출시 | 성격 | 짝꿍 Ubuntu | EOL |
|---|---|---|---|---|
| Humble | 2022.05 | LTS | 22.04 | 2027.05 |
| **Jazzy** | 2024.05 | **LTS** | **24.04** | **2029.05** |

- distro는 특정 Ubuntu에 바인딩 (우리 컨테이너 = 24.04 → Jazzy)
- 현업 로봇 상당수는 아직 Humble (로봇은 출하 후 OS를 안 바꾼다)
- 우리가 Jazzy인 이유: 최신 LTS(2029년까지), arm64 Tier 1, 신형 Gazebo(Harmonic) 짝꿍

### 3-2. 레이어 스택 — rcl vs rclcpp

```
내 코드
└─ rclcpp / rclpy        ← 언어별 SDK (사용자 API)
   └─ rcl                ← C로 된 공통 코어 (언어 중립)
      └─ rmw             ← 미들웨어 추상화 인터페이스
         └─ Fast DDS 등  ← 실제 통신 구현체 (교체 가능)
```

- **rcl**: 노드·pub/sub·서비스·타이머·파라미터 로직을 순수 C로 구현. C인 이유: 모든 언어가 C를 바인딩할 수 있어서 (libcurl을 온갖 언어가 감싸는 것과 같은 설계)
- **rclcpp**: rcl의 C++ 래퍼 + C++만의 가치 — RAII 수명 관리(SharedPtr), 템플릿 타입 안전, 람다 콜백, 그리고 **executor**(`spin()`의 실체, 콜백 스케줄링; rcl에 없고 언어 레이어마다 구현)
- C++ talker와 Python listener가 그냥 호환되는 이유: 밑층(rcl→DDS)이 같아서
- **rmw 교체 가능성**: `-DDEFAULT_RMW_IMPLEMENTATION=rmw_fastrtps_cpp` = 기본 미들웨어가 Fast DDS라는 뜻. Cyclone DDS나 Zenoh로 갈아껴도 내 코드는 불변 (JDBC 아래 드라이버 교체와 같은 그림)
- 참고: rclc = 마이크로컨트롤러용 순수 C 클라이언트 (micro-ROS에서 사용)

### 3-3. package.xml — 도구들을 위한 선언

빌드 스크립트가 아니라 **메타데이터**. build.gradle의 dependencies 블록에 해당.

| 태그 | 의미 | Gradle 대응 |
|---|---|---|
| `<buildtool_depend>ament_cmake` | 빌드 도구 자체 | Gradle 플러그인 선언 |
| `<depend>` | 일반 의존성 (아래 셋 합본) | `implementation` |
| `<build_depend>` | 컴파일 때만 | `compileOnly` |
| `<exec_depend>` | 실행 때만 | `runtimeOnly` |
| `<test_depend>` | 테스트만 | `testImplementation` |
| `<export><build_type>` | colcon에게 빌드 백엔드 알림 | java vs kotlin 플러그인 선택 |

읽는 소비자가 둘:
1. **colcon** — 워크스페이스 빌드 순서 결정
2. **rosdep** — `rosdep install --from-paths src`로 미설치 의존성을 apt로 자동 설치 (새 머신/CI 부트스트랩)

⚠️ 함정: package.xml의 depend는 **링크를 안 해준다.** 실제 컴파일·링크는 CMakeLists의 `find_package` + `ament_target_dependencies`. 두 목록은 수동 동기화 — "내 머신에선 되는데 CI에서 깨짐"의 단골 원인.

### 3-4. ament_cmake — CMake 위의 ROS 규약 매크로

별도 빌드 시스템이 아니라 **100% CMake로 작성된 확장** (`/opt/ros/jazzy/share/ament_cmake_core/cmake/`에서 직접 읽을 수 있음).

- `ament_target_dependencies(타깃 rclcpp std_msgs)`: 패키지 이름만 대면 인클루드+링크+플래그를 재귀로 한 번에 연결
- `ament_package()` (항상 마지막 줄)가 하는 4가지:
  1. package.xml 읽기·검증
  2. CMake config 생성·설치 → 다른 패키지가 `find_package(내패키지)` 가능해짐
  3. **ament index 등록** (`install/.../ament_index/...`) → `ros2 run`/`ros2 pkg list`가 패키지를 발견하는 명부
  4. 환경 훅 설치 (package.bash 등) → setup.bash 체이닝의 재료
- ament 없이 순수 CMake로도 컴파일은 된다. 하지만 ros2 run이 못 찾고, find_package도 안 되고, setup.bash 체이닝도 없다 — ament_package()는 그 보일러플레이트의 자동화
- Gradle 번역: CMake = Gradle 코어, ament_cmake = `java-library` + `maven-publish` 플러그인
- 어원: 식물학에서 ament와 catkin(ROS 1 빌드 시스템)은 동의어(버들강아지 꽃차례) — 후계자에게 동의어 이름을 붙인 것

### 3-5. `source install/setup.bash` — 워크스페이스를 ROS 세계에 등록

- **어디서 왔나**: colcon build 성공 시 install/에 자동 생성
- **뭘 하나**: `AMENT_PREFIX_PATH`(ROS의 패키지 검색 경로) 등에 install/을 추가. `ros2 run pkg exe`는 그 경로들에서 `lib/pkg/exe`를 찾는 것뿐
- 안 하면: `Package 'xxx' not found` (ROS 초보 에러 부동의 1위)
- **왜 source인가**: 환경변수는 셸 프로세스 로컬이라, 자식이 아닌 **현재 셸**에서 실행해야 함. 새 터미널마다 다시 필요. Python venv activate와 정확히 같은 패턴
- **오버레이 구조**: `/opt/ros/jazzy/setup.bash` = 언더레이(ROS 본체), `install/setup.bash` = 오버레이(내 워크스페이스). 오버레이는 자기가 빌드될 때의 언더레이를 기억해 함께 등록 → 새 터미널에선 오버레이 하나만 source하면 됨
- 자동화(선택): `.bashrc`에 `[ -f /ws/ros2_ws/install/setup.bash ] && source /ws/ros2_ws/install/setup.bash`

---

## 4. 문화·역사 상식

### GNU — "GNU's Not Unix" (재귀 약자)

1983년 리처드 스톨만이 시작한 **자유 소프트웨어 운동/프로젝트**. free = 공짜가 아니라 자유(실행·연구·수정·재배포).

- GNU가 만든 것 = 우리의 일상: bash, gcc/g++, make, gdb, coreutils(ls, cp, grep...), glibc
- 커널(Hurd)만 늦어졌고 그 자리를 1991년 Linux 커널이 채움 → "리눅스" = 정확히는 GNU 도구 + Linux 커널 (그래서 GNU/Linux)
- 라이선스 실무: **GPL은 카피레프트**(가져다 쓰면 내 코드도 같은 자유로 공개) → 로봇 회사가 상용 제품에 GPL 링크를 극도로 조심하는 이유. 반면 MIT(이 레포)·Apache 2.0(ROS 2)은 공개 의무 없는 관대한 라이선스라 기업 친화적
- 이 문화가 없었으면 ROS도, Nav2 소스를 읽고 기여하는 것도 없었다

### LLVM vs vLLM vs LLM — 이름만 사촌

| 이름 | 정체 |
|---|---|
| LLM | 대규모 언어 모델 (AI 모델 자체) |
| vLLM | LLM **추론 서빙 엔진** (2023, PagedAttention) |
| **LLVM** | **컴파일러 인프라 프로젝트** (2000~) |

LLVM 구조: 프론트엔드(clang 등) → LLVM IR → 백엔드(x86/ARM/...). 이 모듈성 덕에 Rust, Swift, Julia, Zig가 전부 LLVM 위에 지어졌다.
**clang 붙은 도구는 전부 LLVM 집안**: clang(컴파일러), clangd(언어 서버), clang-format, clang-tidy, lldb.
즉 이 레포 환경은 "빌드는 GNU네(g++), 코드 이해는 LLVM네(clangd)"의 협업이다.
(딥러닝 컴파일러 세계 — torch.compile, Triton, TVM — 가 LLVM/MLIR을 부품으로 쓰긴 하므로 "AI에서 LLVM을 들었다"가 완전 헛기억은 아님.)

---

## 5. Chapter A에서 실제로 밟은 지뢰 요약

상세한 경위와 감정은 일기(journal/) 참고. 여기엔 증상→원인만.

| 증상 | 진짜 원인 | 검색 키워드 |
|---|---|---|
| 이미지가 조용히 느리게 돎 | osrf 데스크톱 이미지는 amd64 전용 → Apple Silicon에서 에뮬레이션 | docker apple silicon amd64 emulation ros |
| `configure_file Problem configuring file` (ament_package_xml) | **파일명이 `" package.xml"`** — 앞에 공백 | — |
| `XML or text declaration not at start of entity: line 1, column 1` | `<?xml` 앞의 공백 1바이트 (XML 선언은 파일 첫 바이트여야) | ros2 package.xml invalid XML line 1 |
| 코드가 화면에 있는데 빌드 결과 0바이트 | **에디터 미저장** (탭의 ● 점, "1 unsaved") | — |
| 빌드 성공 후에도 빨간 물결 | clangd가 새 일지를 안 읽음 → Restart language server | clangd restart compile_commands |

교훈 한 줄: **에러 메시지는 증상을 말하고, 원인은 종종 다른 곳에 있다.** 보이지 않는 문자(공백, 미저장)가 사흘치 메시지를 만들 수 있다.
