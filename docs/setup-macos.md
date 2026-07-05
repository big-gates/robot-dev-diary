# 개발 환경 — macOS (Apple Silicon)

기준 머신: Apple Silicon, RAM 32GB, macOS 14.x, Docker Desktop 설치됨.
결론부터: **ROS 2는 macOS 네이티브 지원이 없다.** 컨테이너(Ubuntu 24.04 + ROS 2 Jazzy)를 기본 무대로 쓰고, GUI(RViz/Gazebo)는 Foxglove 또는 noVNC로 우회한다.

## 1. IDE — 두 가지 경로 (IntelliJ IDEA는 불가)

IntelliJ IDEA는 C++을 지원하지 않는다(공식 C++ 플러그인 없음). 선택지는 둘이고, 환경 자체(컨테이너 + colcon + compile_commands.json)는 IDE 불가지론적이라 **언제든 갈아탈 수 있다.**

### 경로 A — VS Code + Dev Containers (ROS 커뮤니티 표준)

컨테이너 *안에서* 에디터가 통째로 열린다 — 터미널, IntelliSense, 디버거, 확장 전부 리눅스 컨테이너 기준으로 동작. macOS에서 ROS 2를 할 때 가장 마찰이 적은 조합이다.

1. `code` 명령이 PATH에 없으면: VS Code에서 `Cmd+Shift+P` → "Shell Command: Install 'code' command in PATH"
2. 확장 설치:
   - **Dev Containers** (`ms-vscode-remote.remote-containers`) — 핵심
   - **clangd** (`llvm-vs-code-extensions.vscode-clangd`) — C++ 인텔리센스 (colcon의 compile_commands.json을 그대로 먹는다)
   - **CMake Tools** (`ms-vscode.cmake-tools`) — Part 1의 순수 CMake 챕터용
   - (선택) C/C++ (`ms-vscode.cpptools`) — 디버깅(gdb)용
3. 레포 루트에 `.devcontainer/devcontainer.json` 작성 (Chapter A 과제 — 직접 타이핑):

```json
{
  "name": "ros2-jazzy",
  "build": { "dockerfile": "Dockerfile" },
  "workspaceMount": "source=${localWorkspaceFolder},target=/ws,type=bind",
  "workspaceFolder": "/ws",
  "customizations": {
    "vscode": {
      "extensions": [
        "llvm-vs-code-extensions.vscode-clangd",
        "ms-vscode.cmake-tools"
      ]
    }
  }
}
```

같은 폴더의 `Dockerfile`은 아래 2절 참고 — **베이스 이미지는 `ros:jazzy`를 쓴다** (osrf 데스크톱 이미지는 amd64 전용이라 Apple Silicon에서 에뮬레이션으로 돈다).

4. `Cmd+Shift+P` → "Dev Containers: Reopen in Container" → 컨테이너 안에서 `colcon build --cmake-args -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`
5. clangd가 빌드 DB를 찾도록 레포 루트에 `.clangd` 작성:

```yaml
CompileFlags:
  CompilationDatabase: build/
```

### 경로 B — CLion (JetBrains 근육 기억이 소중하다면)

JetBrains의 C++ IDE. **2024년부터 비상업 용도 무료**, 단축키·UX는 IDEA와 동일. 리팩토링·코드 탐색 깊이는 여전히 CLion이 위다.

```bash
brew install --cask clion
```

첫 실행 시 JetBrains 계정 로그인 → "Non-commercial use" 라이선스 선택. 컨테이너 연동은 아래 4절(compile_commands / Docker toolchain) 참고.

> 추천: 이미 깔려 있는 VS Code(경로 A)로 시작하고, 대규모 리팩토링이 아쉬워지는 시점(Part 3+)에 CLion을 얹어서 비교해보라. 그 비교도 일기 소재다.

## 2. ROS 2 Jazzy 컨테이너

> **Apple Silicon 함정 (실측):** `osrf/ros:jazzy-desktop`(RViz 포함 데스크톱 이미지)은 **amd64 전용**이다. Apple Silicon에서 pull하면 조용히 amd64가 내려와 에뮬레이션으로 돌고, 노드는 굴러가지만 GUI·시뮬레이션이 심하게 느려진다. `docker image inspect <이미지> --format '{{.Architecture}}'`로 반드시 확인할 것.

해법: 멀티아치(arm64 지원)인 공식 베이스 `ros:jazzy` 위에 데스크톱 패키지를 apt로 얹는다. Ubuntu Noble arm64는 ROS 2 Jazzy Tier 1 플랫폼이라 바이너리 패키지가 전부 있다.

```bash
docker pull ros:jazzy                     # 멀티아치 — Apple Silicon에서 arm64로 내려온다
```

`.devcontainer/Dockerfile`에서 데스크톱 도구를 추가한다:

```dockerfile
FROM ros:jazzy

RUN apt-get update && apt-get install -y --no-install-recommends \
      ros-jazzy-desktop \
      gdb \
      clangd \
    && rm -rf /var/lib/apt/lists/*

RUN echo 'source /opt/ros/jazzy/setup.bash' >> /root/.bashrc
```

첫 "Reopen in Container" 때 이 레이어를 빌드하느라 몇 분 걸린다(1회성, 이후 캐시).

워크스페이스는 이 레포를 통째로 마운트한다:

```bash
docker run -it --name rosdev \
  -v ~/IdeaProjects/robot-dev-diary:/ws \
  -w /ws \
  --network host \
  osrf/ros:jazzy-desktop
```

컨테이너 안에서:

```bash
source /opt/ros/jazzy/setup.bash
colcon build --cmake-args -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

> Chapter A에서 이 명령들을 `Dockerfile` + `scripts/dev.sh`로 고정해 "새 맥북에서 30분 재현"을 달성하는 것이 과제다. 이미지 태그·패키지는 pull 시점에 확인.

## 3. GUI 전략 (RViz · Gazebo)

macOS에서 X11 포워딩(XQuartz)은 느려서 비추. 두 가지 경로:

**A. 헤드리스 + Foxglove (권장, 가장 쾌적)**
- 컨테이너에서 시뮬·노드는 헤드리스로 돌리고, `foxglove_bridge`(websocket)를 띄운다.
- 맥에는 Foxglove 앱을 네이티브 설치: `brew install --cask foxglove-studio`
- 3D 뷰, TF 트리, 토픽 플롯 전부 가능 — RViz 대부분의 용도를 대체.

**B. noVNC 데스크톱 컨테이너 (RViz/Gazebo GUI가 꼭 필요할 때)**
- `tiryoh/ros2-desktop-vnc:jazzy` 같은 커뮤니티 이미지를 쓰면 브라우저(localhost:6080)로 Ubuntu 데스크톱이 뜬다. arm64 지원 여부는 태그 확인.
- Gazebo GUI를 본격적으로 쓰는 J장부터 유용하다.

**C. 성능이 아쉬워지면 — UTM 가상머신**
- Gazebo를 무겁게 돌리는 Part 3+에서 컨테이너가 답답하면 UTM(무료)에 Ubuntu 24.04 arm64를 깔고 그 안에서 전부 네이티브로 돌리는 게 낫다. virtio-gpu로 GL 가속도 어느 정도 받는다. RAM 32GB면 VM에 16GB 줘도 여유.

## 4. CLion ↔ 컨테이너 연동

colcon 빌드가 만든 `compile_commands.json`으로 인덱싱한다:

1. 컨테이너에서 `colcon build --cmake-args -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`
2. `build/` 아래 compile_commands.json이 생김 (마운트 폴더라 맥에서도 보임)
3. CLion → Open → compile_commands.json 선택 → "Open as Project" (Compilation Database 프로젝트)
4. 경로가 컨테이너 기준(`/ws`, `/opt/ros/...`)이므로 CLion의 remote/컨테이너 toolchain 또는 Dev Container 기능으로 맞춘다. CLion의 Docker toolchain(Settings → Build → Toolchains → Docker)을 쓰면 헤더 해석까지 컨테이너 기준으로 된다.

> 처음엔 이 연동이 가장 큰 삽질 포인트다. 그 삽질이 Chapter A의 일기 소재다 — 기록해두면 뒤따라오는 맥 유저들이 가장 고마워할 문서가 된다.

## 5. 실물 로봇 단계(Chapter T)의 환경

실물은 어차피 SBC(Jetson Orin Nano / Raspberry Pi 5) 위 Ubuntu다. 그때는 맥에서 SSH 원격 개발(CLion Remote Development / VS Code Remote-SSH)로 전환하면 되고, 컨테이너에서 익힌 워크플로가 그대로 이식된다.
