#include "robomath/line.hpp"
#include <gtest/gtest.h>
#include <robomath/angles.hpp>
#include <robomath/vec2.hpp>
#include <vector>

using namespace robomath;

TEST(Line, PerfectFit) {
  std::vector<Vec2> pts = {{0.0, 1.0}, {1.0, 3.0}, {2.0, 5.0}};
  auto l = fit_line(pts);
  ASSERT_TRUE(l.has_value());
  EXPECT_NEAR(l->slope, 2.0, 1e-9);
  EXPECT_NEAR(l->intercept, 1.0, 1e-9);
}

// 노이즈가 낀 현실 상황: y≈2 수평 벽에 위아래로 흔들리는 점들.
// 대칭 노이즈라 최소제곱이 상쇄 → slope≈0, intercept≈2.
// 완벽한 직선이 아니므로 tolerance를 넉넉히(0.1) 준다.
TEST(Line, NoisyHorizontalWall) {
  std::vector<Vec2> pts = {
      {0.0, 2.1}, {1.0, 1.9}, {2.0, 2.0}, {3.0, 1.9}, {4.0, 2.1}};
  auto l = fit_line(pts);
  ASSERT_TRUE(l.has_value());
  EXPECT_NEAR(l->slope, 0.0, 0.1);
  EXPECT_NEAR(l->intercept, 2.0, 0.1);
}

// 점이 2개 미만이면 직선을 정의할 수 없다 → nullopt 가드 검증.
TEST(Line, TooFewPoints) {
  std::vector<Vec2> pts = {{1.0, 1.0}};
  auto l = fit_line(pts);
  EXPECT_FALSE(l.has_value());
}

TEST(Line, SimpleCross) {
  Line l1 = Line{1, 0};
  Line l2 = Line{-1, 2};

  auto v = intersect(l1, l2);
  ASSERT_TRUE(v.has_value()); // 값 있는지 먼저 (없으면 아래서 죽음)
  EXPECT_NEAR(v->x, 1.0, 1e-9); // optional은 -> 로 꺼낸다
  EXPECT_NEAR(v->y, 1.0, 1e-9);
}

TEST(Line, ParallelNoIntersect) {
  Line l1 = Line{2, 1};
  Line l2 = Line{2, 5};
  auto v = intersect(l1, l2);
  EXPECT_FALSE(v.has_value());
}