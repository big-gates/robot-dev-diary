# Chapter K 학습 노트 — TF2 프레임 트리: 왜 map → odom 이 따로 있는가

> "오도메트리는 부드럽지만 틀리고, 위치추정은 정확하지만 튄다."
> 이 둘을 화해시키는 구조가 `map → odom → base_link` 3단 트리다.
> 환경: ROS 2 Jazzy, `tf2_ros` broadcaster/listener, 2D 차동구동 가짜 로봇.

## 0. 결론 먼저 — 표준 프레임 트리

```text
map ──────▶ odom ──────▶ base_link ──────▶ laser
    보정값        오도메트리         센서 마운트
```

| 변환 | 누가 발행 | 성격 | 주기 |
|---|---|---|---|
| `map → odom` | 위치추정 (AMCL 등) | 누적 오차 **보정값**. 갱신될 때 점프 | 낮음 (수 Hz) |
| `odom → base_link` | **오도메트리 노드 (내가 만든 것)** | 연속적, 부드러움. **드리프트함** | 높음 (수십 Hz) |
| `base_link → laser` | static transform | **고정**. 로봇 조립 시 결정 | 한 번 (latched) |

- 동적 변환은 `/tf` 토픽, QoS `VOLATILE`
- 정적 변환은 `/tf_static` 토픽, QoS **`TRANSIENT_LOCAL`** — 늦게 접속한 노드도 저장된 값을 받는다
  (static 은 한 번만 보내므로 `VOLATILE` 이면 나중에 켜진 노드가 영영 모르게 된다)

## 1. 왜 3단인가 — 두 정보원의 결함

| 정보원 | 장점 | 결함 |
|---|---|---|
| 오도메트리 (바퀴 엔코더) | 연속적, 부드러움, 고주파 | **드리프트** — 오차가 누적된다 |
| 위치추정 (스캔 매칭) | 지도 기준 **절대 위치** | 저주파, 갱신 시 **점프** |

H장에서 오도메트리 드리프트를 직접 관찰했다. 바퀴가 미끄러지거나 `track` 값이 조금만 틀려도
오차가 쌓인다. 그렇다고 위치추정 결과만 쓰면 로봇 위치가 뚝뚝 끊긴다.

**둘 다 필요한데 성질이 정반대다.** 3단 트리는 각각을 자기 자리에 둔다.

## 2. 숫자로 보기

```text
[시각 0] 출발
  실제 위치        (0.00, 0.00)
  odom → base_link (0.00, 0.00)
  map → odom       항등 (0, 0)        보정 없음
  map → base_link  (0.00, 0.00)   OK

[시각 T] 10 m 주행 후 — 바퀴가 미끄러져 오도메트리가 틀어짐
  실제 위치        (9.80, 0.10)      <- 위치추정이 스캔 매칭으로 알아냄
  odom → base_link (10.00, 0.00)     <- 여전히 부드럽게 증가 중
  map → odom       (-0.20, 0.10)     <- "오도메트리가 이만큼 틀렸다"
  map → base_link  (9.80, 0.10)  OK  <- 두 변환의 합성
```

### 핵심 통찰

> **`map → odom` 은 "오도메트리가 그동안 얼마나 틀렸나"이다.**
> 누적 오차 그 자체가 프레임 관계로 표현된 것.

로봇이 정확할수록 이 변환은 항등에 가깝고, 드리프트가 쌓일수록 커진다.

## 3. 왜 `map → base_link` 를 직접 발행하면 안 되나

### 이유 (1) — TF 트리는 부모가 하나여야 한다

```text
        map            odom
          \             /
           v           v
            base_link          <- 부모가 둘! 트리가 아니다
```

TF 는 **트리**다. 프레임마다 부모가 정확히 하나여야 두 지점 사이 경로가 유일하게 정해진다.
부모가 둘이면 `map → laser` 를 물었을 때 어느 길로 가야 할지 알 수 없다.

### 이유 (2) — 부드러움이 사라진다

위치추정이 `map → base_link` 를 직접 발행하면, 로봇 위치가 **그 갱신 주기로만** 바뀌고
갱신마다 점프한다. 그 위에서 경로 추종 제어기를 돌리면 **위치가 튈 때마다 제어 출력도 튄다.**

보정을 `odom` 프레임에 걸면 `odom → base_link` 는 그대로 부드럽게 유지되고,
보정은 그 아래에서 조용히 일어난다.

## 4. 그래서 어디에 뭘 쓰나

| 용도 | 기준 프레임 | 이유 |
|---|---|---|
| 지역 제어 — 장애물 회피, 경로 추종 | **`odom`** | 점프하면 안 된다. 짧은 시간이라 드리프트도 무시 가능 |
| 전역 목표 — "3번 테이블로 가라" | **`map`** | 절대 위치가 중요. 드리프트가 보정돼야 한다 |

Nav2 가 정확히 이렇게 나뉘어 있다 — `local_costmap` 의 `global_frame` 은 `odom`,
`global_costmap` 은 `map`. 파라미터 파일의 그 두 줄이 이 구조의 결과다.

## 5. 내 `libpose2d` 가 은퇴한 지점

H장 문제 3-3 에서 손으로 짠 2단 체인:

```cpp
Vec2 sensor_to_map(const Pose2D &robot, const Pose2D &mount, Vec2 sensed);
//                       ^                    ^
//               odom→base_link         base_link→laser
```

TF2 에서는 두 변환을 **각자 발행**해두면 합성은 자동이다.

```text
laser 기준 (1.0, 0, 0) 을 odom 기준으로:
  buffer_->transform(point, "odom")  ->  (1.20, 0.00, 0.10)

검산: 측정 1.0 + 마운트 0.2 = 1.20, 높이 0.10
로봇이 0.75 로 이동하면 -> (1.95, 0.00, 0.10)  = 0.75 + 0.2 + 1.0
```

| | 손으로 짠 `sensor_to_map` | TF2 |
|---|---|---|
| 2단 체인 | 함수 하나 | 조회 한 줄 |
| **3단, 5단으로 늘어나면** | **곱셈을 추가해야 함** | **코드 그대로** |
| 시간 보간 | 없음 | 있음 |
| 여러 노드가 나눠 발행 | 불가 | 가능 |

직접 만들어봤기 때문에 저 한 줄이 무슨 곱셈을 대신하는지 아는 상태로 쓰게 된다.

## 6. TF 디버깅 도구

```bash
ros2 run tf2_ros tf2_echo <부모> <자식>     # 실시간 변환 출력
ros2 run tf2_tools view_frames               # 트리를 PDF/GV 로
ros2 topic echo /tf                          # 동적 변환 날것
ros2 topic echo /tf_static                   # 정적 변환 날것
ros2 topic info /tf --verbose                # QoS 확인
```

- `view_frames` 결과 파일은 **`frames_<타임스탬프>.pdf` / `.gv`** 다. `frames.pdf` 가 아니라서
  못 찾기 쉽다. `-o 이름` 으로 고정할 수 있다
- `.gv` 는 텍스트라 뷰어 없이 읽힌다. 컨테이너에서는 이쪽이 편하다
- `/ws` 가 호스트와 bind mount 이므로, `cd /ws` 후 실행하면 맥에서 바로 PDF 를 열 수 있다

### `.gv` 가 알려주는 것

```text
odom → base_link:   Average rate 2.222     Buffer length 4.501
base_link → laser:  Average rate 10000.0   Buffer length 0.0
```

- `rate 2.222` = 500 ms 타이머와 일치. **발행 주기 검증**에 쓸 수 있다
- `Buffer length 4.501` = TF 가 4.5 초치 과거를 보관 중 → **시간 보간의 근거**
- `rate 10000.0`, `Buffer 0.0` = **static transform 의 표식**. 시간과 무관하게 항상 유효

## 7. TF 조회 원칙

```text
조회는 항상 실패할 수 있다.
```

`active` 상태여도 실패하는 경우가 있다.

- **첫 tick** — 방금 발행한 TF 가 아직 Buffer 에 안 들어옴 (구독은 비동기)
- **시간 불일치** — 요청한 시각의 변환이 아직 없음
- 프레임 이름 오타, 트리가 끊김

그래서 `try` / `catch (const tf2::TransformException &ex)` 는 선택이 아니라 필수이고,
`ex.what()` 을 반드시 로그에 남겨야 한다. 실패 이유가 전부 다른 메시지로 나오기 때문이다.

```text
"odom" passed to lookupTransform argument target_frame does not exist.
```

이 메시지는 "그런 프레임이 트리에 등록된 적이 없다"는 뜻이다.
프레임 이름을 의심하기 전에 **`ros2 topic info /tf` 의 `Publisher count`** 를 먼저 보는 게 빠르다.
0 이면 아무도 발행하지 않고 있다는 뜻이다.

## 8. 이번에 걸린 함정

- **`translation.z` 에 `theta` 를 넣음** — 2D 라서 `(x, y, theta)` 세 개라고 생각하기 쉽지만
  TF 는 3D 다. 위치 3개 + 회전 4개(quaternion)이고, `theta` 는 z축 **회전**이지 z축 **위치**가
  아니다. 제자리 회전만 시켰는데 로봇이 2.25 m 공중에 떴다
- **`TransformListener` 를 인자 없이 생성** — Buffer 없이는 존재 이유가 없어서 기본 생성자가 없다.
  `(*buffer_, this)` 로 Buffer 참조와 노드를 함께 넘긴다. 노드를 안 넘기면 **몰래 노드를 하나
  더 만든다**
- **해제 순서** — listener 가 buffer 를 **참조**로 들고 있으므로 listener 를 먼저 해제해야 한다.
  만든 순서의 역순
- **`on_configure` 에서 조회 시도** — 그 시점엔 TF 트리가 비어 있다. 조회는 TF 가 흐르는
  `on_timer` 에서
- **좀비 프로세스** — CLI `static_transform_publisher` 를 끄지 않아서 "코드가 되는 것처럼" 보였다.
  `ps -eo comm | grep -c fake_robot_node` 가 진실. `ros2 node list` 는 `kill -9` 로 죽인 노드를
  약 30초간 유령으로 보여준다
