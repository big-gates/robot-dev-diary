// my-turtlesim v1 — LiDAR 인지 파이프라인 시각화
//
//   진짜 벽(정답)  →  가짜 LiDAR 점(노이즈)  →  fit_line로 직선 복원
//                  →  intersect로 코너 복원  →  SVG로 그리기
//
// 우리가 Set 2에서 만든 robomath(Vec2, fit_line, intersect, project_forward)를
// 하나도 빠짐없이 실제로 굴려서, "코드가 눈에 보이는" 순간을 만든다.

#include "svg_canvas.hpp"

#include <robomath/line.hpp>
#include <robomath/vec2.hpp>

#include <cmath>
#include <fstream>
#include <iostream>
#include <optional>
#include <random>
#include <vector>

using namespace robomath;

// 벽(선분) a→b를 따라 n개 점을 균등 샘플하고 각 점에 sigma(m) 가우시안 노이즈.
// = "가짜 LiDAR 스캔": 실제 센서처럼 점들이 벽 근처에 흩어진다.
static std::vector<Vec2> fake_scan(Vec2 a, Vec2 b, int n, double sigma,
                                   std::mt19937 &rng) {
  std::normal_distribution<double> noise(0.0, sigma);
  std::vector<Vec2> pts;
  for (int i = 0; i < n; ++i) {
    double t = static_cast<double>(i) / (n - 1); // 0 → 1
    Vec2 on_wall = a + (b - a) * t;               // 벽 위 진짜 점
    pts.push_back(Vec2{on_wall.x + noise(rng), on_wall.y + noise(rng)});
  }
  return pts;
}

// 무한 직선 y=ax+b 위에서 특정 x의 점 — 그리기용 끝점을 뽑는다.
static Vec2 line_at(const Line &l, double x) {
  return Vec2{x, l.slope * x + l.intercept};
}

int main() {
  // ── 1. 그라운드 트루스: 코너를 이루는 두 벽 (둘 다 수직이 아님!) ──
  const Vec2 wallA_a{0.5, 1.0}, wallA_b{5.5, 1.3}; // 바닥 벽 (거의 수평)
  const Vec2 wallB_a{5.5, 1.3}, wallB_b{3.5, 3.5}; // 오른쪽 경사 벽
  const Vec2 true_corner = wallA_b;                 // 두 벽이 만나는 진짜 코너

  // ── 2. 가짜 LiDAR 스캔 (고정 시드 → 매번 똑같은 그림) ──
  std::mt19937 rng(42);
  const double sigma = 0.04; // 4cm 노이즈
  std::vector<Vec2> scanA = fake_scan(wallA_a, wallA_b, 12, sigma, rng);
  std::vector<Vec2> scanB = fake_scan(wallB_a, wallB_b, 12, sigma, rng);

  // ── 3. 인지: 흩어진 점 → 직선 (최소제곱) ──
  std::optional<Line> lineA = fit_line(scanA);
  std::optional<Line> lineB = fit_line(scanB);

  // ── 4. 코너 복원: 두 직선의 교점 ──
  std::optional<Vec2> corner;
  if (lineA && lineB) {
    corner = intersect(*lineA, *lineB);
  }

  // ── 5. 관찰자(로봇) 포즈 — 장식용. project_forward로 삼각형 3점 생성 ──
  const double rx = 1.2, ry = 3.2;
  const double heading = std::atan2(1.4 - ry, 4.0 - rx); // 장면 중앙을 바라봄
  const Vec2 tip = project_forward(rx, ry, heading, 0.20);
  const Vec2 back_l = project_forward(rx, ry, heading + 2.5, 0.15);
  const Vec2 back_r = project_forward(rx, ry, heading - 2.5, 0.15);

  // ── 6. 그리기 ──
  viz::SvgCanvas cv(6.0, 4.0, 110.0); // 6m x 4m 공간

  // 6a. 진짜 벽 (희미한 굵은 회색 = 정답)
  cv.line(wallA_a, wallA_b, "#3a3f4b", 8.0);
  cv.line(wallB_a, wallB_b, "#3a3f4b", 8.0);

  // 6b. LiDAR 광선 (로봇 → 각 점, 아주 희미하게 — 스캔처럼 보이게)
  for (const Vec2 &p : scanA) {
    cv.line(Vec2{rx, ry}, p, "#e74c3c33", 0.6);
  }
  for (const Vec2 &p : scanB) {
    cv.line(Vec2{rx, ry}, p, "#3498db33", 0.6);
  }

  // 6c. 스캔 점 (센서가 실제로 받은 것)
  for (const Vec2 &p : scanA) {
    cv.dot(p, 3.5, "#e74c3c"); // 빨강
  }
  for (const Vec2 &p : scanB) {
    cv.dot(p, 3.5, "#3498db"); // 파랑
  }

  // 6d. 피팅된 직선 (점 구름에서 복원한 벽, 점선)
  if (lineA) {
    cv.line(line_at(*lineA, 0.0), line_at(*lineA, 6.0), "#e74c3c", 2.0, "8 4");
  }
  if (lineB) {
    // 경사 벽은 x범위를 좁게 (가파르면 화면 밖으로 치솟음)
    cv.line(line_at(*lineB, 3.0), line_at(*lineB, 5.8), "#3498db", 2.0, "8 4");
  }

  // 6e. 진짜 코너(회색 원) vs 복원한 코너(초록 X)
  cv.dot(true_corner, 5.0, "#7f8c8d");
  if (corner) {
    cv.cross(*corner, 8.0, "#2ecc71", 3.0);
  }

  // 6f. 로봇 삼각형
  cv.polygon({tip, back_l, back_r}, "#9b59b6");

  // 6g. 범례
  cv.text(Vec2{0.15, 3.9},
          "gray=truth   dots=LiDAR scan   dashed=fit_line   green X=corner",
          "#95a5a6", 13.0);

  // ── 7. 저장 + 콘솔 요약 ──
  std::ofstream("lidar.svg") << cv.str();
  std::cout << "SVG 저장 완료: lidar.svg\n\n";

  auto report = [](const char *name, const std::optional<Line> &l) {
    if (l) {
      std::cout << "  " << name << " fit : slope=" << l->slope
                << "  intercept=" << l->intercept << "\n";
    } else {
      std::cout << "  " << name << " fit : 실패 (nullopt)\n";
    }
  };
  report("wallA", lineA);
  report("wallB", lineB);

  std::cout << "\n  true corner      : (" << true_corner.x << ", "
            << true_corner.y << ")\n";
  if (corner) {
    std::cout << "  recovered corner : (" << corner->x << ", " << corner->y
              << ")\n";
  } else {
    std::cout << "  recovered corner : 없음\n";
  }
  return 0;
}
