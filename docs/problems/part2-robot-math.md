# Part 2 실전 문제집 — 서빙로봇 현장에서 온 로봇 수학 18문제

> 모든 문제는 실제 상용 서빙로봇/AMR 운영에서 매일 돌아가는 계산이다.
> 문제마다 "현업 맥락"이 붙어 있다 — 이 함수가 실제 로봇 스택 어디에서 뛰고 있는지.
> 챕터 매핑: **Set 1 = F장, Set 2 = G장, Set 3 = H장.**

## 규칙

1. **테스트 먼저(TDD).** 문제에 적힌 테스트 케이스를 GoogleTest로 먼저 옮겨 적고(RED), 구현해서 통과시킨다(GREEN).
2. **std만 사용.** Eigen 등 외부 수학 라이브러리 금지 — 직접 만드는 게 목적이다. (Eigen·TF2와의 비교와 은퇴식은 K장에서)
3. 부동소수 비교는 `EXPECT_NEAR(actual, expected, 1e-9)`. `==` 금지.
4. 코드는 하나의 라이브러리로 **누적**한다: `part2/robomath/` 헤더 + `part2/tests/`. 문제를 풀수록 라이브러리가 자란다 — Set 3이 끝나면 이게 `libpose2d` v1.0이다.
5. 각도의 내부 표현은 **라디안**. 도(degree)는 입출력 경계에서만.

Modern C++ 체크포인트: `constexpr` 함수, `[[nodiscard]]`, `struct` + 연산자 오버로딩, `std::optional`(해 없음 표현), 가상 함수와 인터페이스(HAL), 헤더 온리 설계, `EXPECT_NEAR`와 부동소수 감각.

## 관통 프로젝트: my-turtlesim — 눈에 보이면 수학이 산다

문제는 **테스트로 검증**하고, **시뮬레이터로 확인**한다. D장에서 만든 `RobotHAL`/`SimRobot` 위에 렌더러를 얹어, 세트를 끝낼 때마다 거북이가 새 능력을 얻는다:

- **Set 1 완료 → v0 (터미널 ASCII)**: 화살표 거북이(→↗↑)가 `shortest_turn`으로 최단 방향 회전하고, `bearing_to`로 목표 테이블을 향해 전진한다
- **Set 2 완료 → v1 (SVG 출력)**: 가짜 LiDAR 점들 → 피팅된 벽 직선 → 교점(코너)이 그림 파일로 그려진다
- **Set 3 완료 → v2 (펜 궤적)**: 오도메트리 추정 경로 vs 실제 경로의 드리프트가 두 색 선으로 보이고, 접근 포즈로 다가가 정렬하는 데모까지

렌더링에 외부 라이브러리는 안 쓴다 — ASCII는 `std::cout`, SVG는 그냥 텍스트 파일이다(진짜다, 열어보면 안다). 브레인 코드는 `RobotHAL` 인터페이스만 알고 `SimRobot`을 모른다 — 이 분리가 U장에서 실물 로봇으로 갈아끼울 때의 보험이고, ros2_control이 프로덕션에서 하는 일의 미니어처다. 라이브 GUI가 궁금해지면 그건 M장(Gazebo)과 Foxglove가 해결해준다.

---

## Set 1 — 각도와 헤딩 (삼각함수) · F장

### 1-1. 각도의 기초 체력: 변환과 정규화
**현업 맥락**: 모든 각도 버그의 90%는 도/라디안 혼동과 ±180° 경계에서 터진다. 로봇이 359°에서 1°로 넘어가는 순간 "358° 회전" 명령이 나가는 사고 — 실화다.
```cpp
constexpr double deg2rad(double deg);
constexpr double rad2deg(double rad);
double wrap_to_pi(double rad);   // 반환 범위 (-π, π]
```
**테스트**:
- `deg2rad(180.0)` ≈ π
- `wrap_to_pi(deg2rad(360.0))` ≈ 0
- `wrap_to_pi(deg2rad(-190.0))` ≈ deg2rad(170.0)
- `wrap_to_pi(deg2rad(540.0))` ≈ π  (경계: -π가 아니라 +π)

**힌트**: `std::fmod`는 음수에서 기대와 다르게 동작한다 — 그게 이 문제의 함정 전부다.
**확장**: `struct Radian { double v; };` 타입으로 도/라디안 혼동을 컴파일 타임에 막아보기.

### 1-2. 최단 회전각 — "테이블을 바라봐"
**현업 맥락**: 서빙 도착 후 테이블 정면으로 정렬할 때, 회전은 곧 서빙 시간이다. 항상 최단 방향으로 돌아야 한다.
```cpp
double shortest_turn(double from_rad, double to_rad);  // 반환 (-π, π], 양수 = 반시계
```
**테스트**:
- 350° → 10° : `+20°` (−340°가 아니라!)
- 10° → 350° : `−20°`
- 0° → 180° : `+180°` (정의상 +로 통일)

**힌트**: `wrap_to_pi(to - from)`. 1-1이 맞으면 한 줄이다 — 기초가 무기가 되는 첫 경험.

### 1-3. 베어링 — "저 테이블이 몇 시 방향이지?"
**현업 맥락**: 호출이 오면 로봇은 목적지 테이블의 방향부터 계산한다. 모든 "~를 향해"의 출발점.
```cpp
double bearing_to(double rx, double ry, double tx, double ty);          // 지도 기준 방위각
double relative_bearing(double rx, double ry, double rtheta,
                        double tx, double ty);                          // 로봇 정면 기준
```
**테스트**:
- (0,0)에서 (1,1)의 bearing ≈ 45°
- (0,0)에서 (-1,0)의 bearing ≈ 180°
- 로봇 (2,3,90°), 테이블 (2,5): relative_bearing ≈ 0° (정면)
- 로봇 (0,0,0°), 테이블 (0,-3): relative_bearing ≈ −90° (오른쪽)

**힌트**: 왜 `atan`이 아니라 `atan2`인가 — (−1,−1)과 (1,1)의 차이를 설명할 수 있어야 한다. 이 질문은 로봇 SW 면접 단골이다.

### 1-4. 데드레커닝 한 스텝 — "1초 뒤 나는 어디에 있나"
**현업 맥락**: 헤딩 θ로 거리 d를 직진하면 위치가 어떻게 변하나. 오도메트리 적분의 최소 단위이자, 충돌 예측("이대로 가면 0.5초 뒤 저 사람과 만난다")의 기본 연산.
```cpp
struct Vec2 { double x, y; };
Vec2 project_forward(double x, double y, double theta, double dist);
```
**테스트**:
- (1, 1, 30°)에서 2.0m 전진 → (2.7320508, 2.0)
- (0, 0, 90°)에서 1.0m 전진 → (0, 1)

### 1-5. 자이로 적분 — 드리프트를 목격하라
**현업 맥락**: IMU 각속도를 적분해 헤딩을 추정한다. 실제 로봇의 헤딩은 이 적분값과 (나중에 배울) 보정의 합작품이고, 보정이 왜 필요한지는 드리프트를 직접 봐야 안다.
```cpp
// omega[i]: i번째 샘플의 각속도(rad/s), dt: 샘플 간격(s)
double integrate_heading(double theta0, std::span<const double> omega, double dt);
```
**테스트**:
- θ₀=0, ω = {0.1, 0.1, ..., 0.1} (10개), dt=0.1 → θ ≈ 0.1 rad
- 적분 후에도 반환값은 wrap_to_pi 범위 유지

**실험(일기 소재)**: ω 샘플마다 +0.001의 바이어스(센서 오차)를 넣고 10분치(6000샘플)를 적분해보라. 헤딩이 몇 도 틀어지는가? — "왜 자이로만으로는 안 되는가"의 체감.

### 1-6. 시야각 판정 — 인사 기능의 실제 로직
**현업 맥락**: 서빙로봇의 "지나가는 사람에게 인사" 기능은 정확히 이 함수다 — 감지된 사람이 로봇 전방 시야각 안에 있는가.
```cpp
bool in_fov(double rx, double ry, double rtheta,
            double tx, double ty, double fov_rad);   // fov = 전체 시야각
```
**테스트**:
- 로봇 (0,0,0°), FOV 60°: (2, 0.5) → true (상대 베어링 ≈ 14°)
- 같은 조건: (2, 2) → false (≈ 45° > 반각 30°)
- 같은 조건: (-1, 0) → false (등 뒤)

**힌트**: `|relative_bearing| <= fov/2`. 1-3이 부품이 된다.

---

## Set 2 — 벡터·행렬·피팅 (선형대수) · G장

### 2-1. Vec2를 제대로 만들기 — 이후 전부의 부품
**현업 맥락**: 좌표, 속도, LiDAR 점 — 로봇 코드의 모든 것이 이 타입 위에 선다.
```cpp
struct Vec2 {
  double x{}, y{};
  Vec2 operator+(Vec2) const;  Vec2 operator-(Vec2) const;
  Vec2 operator*(double) const;
  double dot(Vec2) const;      double norm() const;
  Vec2 normalized() const;     // 영벡터면? — 정책을 정하고 문서화하라
};
```
**테스트**:
- (3,4).norm() = 5
- (1,0).dot((0,1)) = 0 (수직)
- (3,4).normalized() = (0.6, 0.8)

**C++ 포인트**: 값 타입, const 멤버 함수, 멤버 브레이스 초기화. "영벡터 normalize" 같은 경계 정책을 스스로 정하는 것이 API 설계의 시작.

### 2-2. 회전행렬 — 회전을 곱셈으로
**현업 맥락**: "θ만큼 회전"을 행렬로 표현하는 순간, 회전들의 연쇄가 곱셈이 되고 되돌리기가 전치가 된다. TF2의 심장.
```cpp
struct Mat2 { double a, b, c, d; /* [[a b],[c d]] */
  Vec2 operator*(Vec2) const;  Mat2 operator*(Mat2) const;
  Mat2 transpose() const;
};
Mat2 rotation(double theta);
```
**테스트**:
- rotation(90°) * (1,0) = (0,1)
- rotation(30°) * rotation(60°) ≈ rotation(90°) (합성 = 각의 합)
- rotation(θ).transpose() * rotation(θ) ≈ 단위행렬 (회전의 역 = 전치)

### 2-3. 동차좌표 — 회전+이동을 한 방에
**현업 맥락**: "회전하고 나서 이동"을 행렬 하나로. SE(2) 변환의 행렬 표현이고, 3D로 가면 4×4가 될 뿐 같은 구조다 (URDF/TF의 내부 표현).
```cpp
struct Mat3 { /* 3x3, 마지막 행 [0 0 1] */ };
Mat3 transform(double tx, double ty, double theta);
Vec2 apply(const Mat3&, Vec2);   // 동차좌표 (x, y, 1)로 곱하기
```
**테스트**:
- transform(1, 2, 90°) 적용 (1,0) → (1, 3)   (회전 먼저: (0,1) → 이동: (1,3))
- transform(0,0,0°) = 단위행렬

### 2-4. 점-선분 최단거리 — 크로스트랙 에러
**현업 맥락**: "로봇이 계획된 경로 중심선에서 얼마나 벗어났나" = 경로 추종 제어기(Pure Pursuit, Q장)의 핵심 입력값. 좁은 복도 통과 판정에도 쓰인다.
```cpp
struct Closest { Vec2 point; double dist; };
Closest closest_on_segment(Vec2 p, Vec2 a, Vec2 b);   // 선분 ab
```
**테스트**:
- p(1,1), 선분 (0,0)-(2,0) → point (1,0), dist 1
- p(3,1), 선분 (0,0)-(2,0) → point (2,0), dist √2   (⚠️ 끝점 클램핑!)
- p(-1,0), 선분 (0,0)-(2,0) → point (0,0), dist 1

**힌트**: 투영 계수 t = (p−a)·(b−a)/|b−a|² 를 [0,1]로 클램프. 클램핑을 빼먹으면 "무한 직선" 거리가 되는데, 그 버그는 시뮬에서 로봇이 경로 밖 유령 지점을 쫓는 모습으로 나타난다.

### 2-5. 최소제곱 직선 피팅 — LiDAR로 벽 찾기
**현업 맥락**: LiDAR 스캔 점 무리에서 "벽"이라는 직선을 추출한다. 벽과 평행 주행, 도킹 정렬, 스캔 매칭의 원형이 전부 이것.
```cpp
struct Line { double slope, intercept; };                 // y = slope·x + intercept
std::optional<Line> fit_line(std::span<const Vec2> pts);  // 점 2개 미만이면 nullopt
```
**테스트**:
- (0,1), (1,3), (2,5) → slope 2, intercept 1 (정확히 y=2x+1 위)
- (0,0), (1,0.1), (2,-0.1), (3,0) → slope ≈ 0 부근 (노이즈 낀 수평 벽)
- 점 1개 → nullopt

**힌트**: 정규방정식 — slope = (nΣxy − ΣxΣy) / (nΣx² − (Σx)²). 유도 과정을 주석으로 남겨라(완료 기준).
**확장(중요)**: 수직 벽(x=상수)은 y=ax+b로 표현 불가. 이 파라미터화의 한계를 일기에 기록 — 실전 스캔 매칭이 (ρ,θ) 극좌표 파라미터화를 쓰는 이유다.

### 2-6. 두 직선의 교점 — 코너 감지
**현업 맥락**: 벽 두 개를 피팅했으면 교점이 곧 코너(기둥 모서리, 통로 입구)다. 2×2 연립방정식을 행렬식으로 푼다.
```cpp
std::optional<Vec2> intersect(Line l1, Line l2);   // 평행이면 nullopt
```
**테스트**:
- y = x, y = −x + 2 → (1, 1)
- y = 2x+1, y = 2x+3 → nullopt (평행)
- 행렬식이 1e-12보다 작으면 평행 취급 (부동소수에서 "정확히 0"은 없다)

---

## Set 3 — SE(2) 변환 체인과 오도메트리 (libpose2d 완성) · H장

### 3-1. Pose2D와 변환 합성 — 좌표계의 문법
**현업 맥락**: "지도에서 로봇이 (3,4,90°)에 있고, 로봇 기준 1m 앞에 장애물" → 장애물의 지도 좌표는? 매핑·인지·계획 전부가 이 한 문장의 반복이다.
```cpp
struct Pose2D {
  double x{}, y{}, theta{};
  Vec2 transform(Vec2 local) const;        // 로봇 좌표 → 지도 좌표
  Pose2D operator*(const Pose2D&) const;   // 변환 합성 (this ∘ other)
};
```
**테스트**:
- Pose(1,1,90°).transform((1,0)) = (1, 2)
- Pose(1,0,90°) * Pose(1,0,0°) = Pose(1, 1, 90°)
- 항등원: Pose(0,0,0°) * P = P

### 3-2. 역변환 — "저게 내 왼쪽인가 오른쪽인가"
**현업 맥락**: 지도 좌표의 장애물/사람을 로봇 관점으로 가져와야 "왼쪽 30cm, 감속" 같은 판단이 된다.
```cpp
Vec2 inverse_transform(const Pose2D& robot, Vec2 map_point);  // 지도 → 로봇 좌표
Pose2D inverse(const Pose2D&);                                // 역변환 자체
```
**테스트**:
- robot (1,1,90°), 지도점 (1,2) → 로봇 좌표 (1, 0) (3-1의 역방향)
- inverse(P) * P ≈ 항등원
- 판정 함수: 로봇 좌표 y > 0 이면 왼쪽 — robot (0,0,0°), 지도점 (1,1) → 왼쪽

### 3-3. 센서 마운트 캘리브레이션 — 변환 2단 체인
**현업 맥락**: LiDAR는 로봇 중심이 아니라 (0.15m 앞, 5° 틀어진) 마운트에 붙어 있다. 스캔점을 지도에 찍으려면 지도←로봇←센서 두 단을 통과해야 한다. URDF의 static transform, TF2 트리가 자동화하는 게 정확히 이 계산이다.
```cpp
// map_point = robot_pose ∘ mount_pose ∘ sensor_point
Vec2 sensor_to_map(const Pose2D& robot, const Pose2D& mount, Vec2 sensed);
```
**테스트**:
- robot (0,0,90°), mount (0.15,0,0°), 센서점 (1,0) → 지도 (0, 1.15)
- mount 회전이 5°일 때 1m 거리 점의 위치 오차 ≈ 8.7cm — "각도 캘리브레이션이 왜 중요한가"를 숫자로

**힌트**: 3-1의 합성 연산자가 있으면 한 줄: `(robot * mount).transform(sensed)`.

### 3-4. 접근 포즈 — 서빙 정지점 계산
**현업 맥락**: "테이블 정면 30cm에서, 테이블을 마주보고 정지" — 서빙 도착 포즈, 충전 도킹 접근 포즈가 전부 이 계산이다. 목적지 포즈에서 접근 포즈를 유도한다.
```cpp
Pose2D approach_pose(const Pose2D& target, double standoff);
// target의 정면(+x) 방향으로 standoff만큼 떨어진 위치, target을 마주보는 헤딩
```
**테스트**:
- 테이블 (5,5,180°), standoff 0.3 → Pose(4.7, 5, 0°) (테이블 정면은 −x 방향, 로봇은 +x를 보며 마주봄)
- 테이블 (0,0,90°), standoff 0.5 → Pose(0, 0.5, −90°)

### 3-5. 차동구동 오도메트리 — 엔코더에서 포즈로
**현업 맥락**: 서빙로봇 바퀴 두 개의 엔코더 증분이 로봇 포즈가 되는 과정. N장(맨손 매핑)과 diff_drive_controller의 심장을 미리 만든다.
```cpp
// dl, dr: 좌/우 바퀴 이동거리(m), track: 바퀴 간격(m)
Pose2D integrate_odom(const Pose2D& p, double dl, double dr, double track);
```
**테스트** (track = 0.4):
- 직진: dl = dr = 0.1 → x가 헤딩 방향으로 0.1 증가, θ 불변
- 제자리 회전: dl = −0.1, dr = +0.1 → 위치 불변, Δθ = 0.5 rad
- 완만한 커브: dl = 0.09, dr = 0.11 → Δθ = 0.05 rad, 전진 0.1m

**확장(일기 소재)**: 위 구현은 "직선 근사"다. 원호(arc) 모델로도 구현해서 큰 Δθ에서 두 모델의 오차를 비교하라 — 적분 주기가 왜 짧아야 하는지의 답.

### 3-6. 격자 세계 — world↔grid와 레이캐스팅
**현업 맥락**: 점유 격자 지도(OccupancyGrid)는 "원점 + 해상도"로 정의된 격자다. 실좌표↔셀 변환과 "레이저가 통과한 셀 나열"은 N장(맨손 매핑)과 costmap의 기초 연산.
```cpp
struct GridSpec { double origin_x, origin_y, resolution; };
struct Cell { int cx, cy; };
Cell world_to_grid(const GridSpec&, Vec2 world);
Vec2 grid_to_world(const GridSpec&, Cell);                  // 셀 중심
std::vector<Cell> raycast(Cell from, Cell to);              // Bresenham
```
**테스트**:
- spec {origin (−5,−5), res 0.05}: world (0,0) → cell (100, 100)
- grid_to_world(world_to_grid(p)) 오차 ≤ res/2
- raycast (0,0)→(3,1): 시작·끝 포함, 셀 개수 = 4, 연속한 셀은 8-이웃 관계

**의미**: 이 문제를 풀면 "레이저 빔이 지나간 셀은 비어 있고, 끝 셀은 차 있다"는 매핑의 한 문장을 코드로 쓸 준비가 끝난 것이다.

---

## 현업 대응표 — 이 문제들은 어디서 뛰고 있나

| 문제 | 실제 스택에서의 자리 |
|---|---|
| 1-1, 1-2 | 모든 회전 명령의 전처리. tf2의 `getYaw` + 정규화 유틸 |
| 1-3, 1-6 | 사람 인사/광고 모드 시야 판정, 목적지 방향 계산 |
| 1-5 | IMU 전처리, robot_localization의 입력 |
| 2-2, 2-3 | TF2 변환의 내부 표현 (3D에선 4×4) |
| 2-4 | Pure Pursuit/DWB의 크로스트랙 에러 |
| 2-5, 2-6 | 스캔 특징 추출, 도킹 정렬, laser 필터 |
| 3-1~3-3 | TF2 트리 그 자체 (map→odom→base_link→laser) |
| 3-4 | Nav2 goal 전처리, 도킹 서버의 접근 포즈 |
| 3-5 | diff_drive_controller의 odometry 발행 |
| 3-6 | OccupancyGrid/costmap 좌표계, N장 맨손 매핑 |

## 완주 후 얻는 것

- 헤더 온리 `libpose2d` v1.0 — Part 3의 K장에서 TF2와 비교하며 명예롭게 은퇴시킨다
- "AMCL이 왜, TF가 왜"를 물었을 때 수식이 아니라 **내가 짠 코드로** 답하는 능력
- 18문제 × (테스트 + 구현 + 현업 맥락) = 블로그 시리즈 3~4편의 재료
