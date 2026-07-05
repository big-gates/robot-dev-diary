#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

constexpr double kPi = 3.14159265358979323846;

// 1. 배터리가 임계값 미만인가? (복귀 판단의 첫 조각)
bool battery_low(int percent, int threshold) { return percent < threshold; }

// 2. 속도 명령을 안전 한계로 자르기 (cmd_vel 안전장치)
double clamp_speed(double v, double lo, double hi) {
  if (v < lo) {
    return lo;
  } else if (v > hi) {
    return hi;
  } else {
    return v;
  }
}

// 3. 도 → 라디안 (문제 1-1의 예고편)
double deg2rad(double deg) { return deg * (kPi / 180); }

// 4. LiDAR 스캔의 평균 거리 (비어 있으면 0.0 — 정책!)
double average_range(const std::vector<double> &scan) {
  if (scan.empty()) {
    return 0.0;
  }

  double sum = 0.0;

  for (double v : scan) {
    sum += v;
  }

  return sum / scan.size();
}

// 5. threshold 이내로 가까운 장애물 개수
int count_close_obstacles(const std::vector<double> &scan, double threshold) {
  int result = 0;
  for (double v : scan) {
    if (v <= threshold) {
      result += 1;
    }
  }

  return result;
}

// 6. 가장 가까운 장애물 거리 (긴급 정지 판단의 입력값)
//    빈 스캔이면 0.0 대신 무한대를 반환 — "장애물 없음 = 무한히 멀다"
double min_range(const std::vector<double> &scan);

// 7. 가장 가까운 장애물의 인덱스 = 방향 (회피할 쪽 결정)
//    빈 스캔이면 -1
int nearest_index(const std::vector<double> &scan);

// 8. 인덱스 [from, to) 구간의 평균 — "전방 ±30° 섹터만 보기"
//    반개구간(from 포함, to 제외)이 C++ 세계의 표준 관례다
double sector_average(const std::vector<double> &scan, int from, int to);

// 9. 이동 평균 필터 — 노이즈 낀 스캔 스무딩 (실제 LiDAR 전처리의 원형)
//    가장자리는 "유효한 이웃만으로" 평균 (창 축소 정책)
std::vector<double> moving_average(const std::vector<double> &scan, int window);

// 10. 전 구간이 threshold보다 멀면 true — "출발해도 되나?"
bool path_clear(const std::vector<double> &scan, double threshold);

int main() {
  assert(battery_low(15, 20) == true);
  assert(battery_low(20, 20) == false); // 경계: "미만"

  assert(clamp_speed(2.5, 0.0, 1.2) == 1.2);
  assert(clamp_speed(-0.5, 0.0, 1.2) == 0.0);
  assert(clamp_speed(0.8, 0.0, 1.2) == 0.8);

  assert(std::abs(deg2rad(180.0) - kPi) < 1e-9); // double은 == 대신 이렇게

  std::vector<double> scan = {1.0, 2.0, 3.0};
  assert(std::abs(average_range(scan) - 2.0) < 1e-9);
  assert(average_range({}) == 0.0); // 빈 스캔 정책 확인

  assert(count_close_obstacles(scan, 1.5) == 1);
  assert(count_close_obstacles(scan, 0.5) == 0);

  std::vector<double> scan2 = {2.0, 0.4, 3.0, 1.5};
  assert(std::abs(min_range(scan2) - 0.4) < 1e-9);
  assert(nearest_index(scan2) == 1);
  assert(nearest_index({}) == -1);
  assert(std::abs(sector_average(scan2, 1, 3) - 1.7) < 1e-9); // (0.4+3.0)/2
  assert(path_clear(scan2, 0.3) == true);
  assert(path_clear(scan2, 0.5) == false); // 0.4가 걸림

  std::vector<double> smoothed = moving_average({1.0, 5.0, 1.0}, 3);
  assert(std::abs(smoothed[1] - 7.0 / 3.0) < 1e-9); // 가운데: (1+5+1)/3
  assert(std::abs(smoothed[0] - 3.0) < 1e-9); // 가장자리: (1+5)/2 — 창 축소

  std::cout << "all tests passed\n";
  return 0;
}