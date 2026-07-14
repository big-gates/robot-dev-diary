#include <cmath>
#include <robomath/angles.hpp>
#include <robomath/vec2.hpp>
#include <gtest/gtest.h>

using namespace robomath;

TEST(Vec2, ProjectForward) {
  Vec2 a = project_forward(1.0, 1.0, deg2rad(30.0), 2.0);
  EXPECT_NEAR(a.x, 2.7320508, 1e-6);
  EXPECT_NEAR(a.y, 2.0, 1e-6);

  Vec2 b = project_forward(0.0, 0.0, deg2rad(90.0), 1.0);
  EXPECT_NEAR(b.x, 0.0, 1e-9); // 북쪽으로 1m → x는 안 변함
  EXPECT_NEAR(b.y, 1.0, 1e-9);
}

TEST(Vec2, ProjectAndBeartingAreInverse) {
  Vec2 a = project_forward(0.0, 0.0, deg2rad(45.0), 3.0);
  EXPECT_NEAR(bearing_to(0.0, 0.0, a.x, a.y), deg2rad(45.0), 1e-9);
}

TEST(Vec2, Arithmetic) {
  Vec2 a{1.0, 2.0}, b{3.0, 4.0};
  Vec2 sum = a + b;
  EXPECT_NEAR(sum.x, 4.0, 1e-9);
  EXPECT_NEAR(sum.y, 6.0, 1e-9);

  Vec2 diff = b - a;
  EXPECT_NEAR(diff.x, 2.0, 1e-9);
  EXPECT_NEAR(diff.y, 2.0, 1e-9);

  Vec2 scaled = a * 3.0;
  EXPECT_NEAR(scaled.x, 3.0, 1e-9);
  EXPECT_NEAR(scaled.y, 6.0, 1e-9);
}

TEST(Vec2, NormAndDot) {
  EXPECT_NEAR((Vec2{3.0, 4.0}).norm(), 5.0, 1e-9);
  EXPECT_NEAR((Vec2{1.0, 0.0}).dot(Vec2{0.0, 1.0}), 0.0, 1e-9);
  EXPECT_NEAR((Vec2{1.0, 0.0}).dot(Vec2{1.0, 0.0}), 1.0, 1e-9);
  EXPECT_NEAR((Vec2{1.0, 0.0}).dot(Vec2{-1.0, 0.0}), -1.0, 1e-9);
}

TEST(Vec2, Normalized) {
    Vec2 n = (Vec2{3.0, 4.0}).normalized();
    EXPECT_NEAR(n.x, 0.6, 1e-9);
    EXPECT_NEAR(n.y, 0.8, 1e-9);
    EXPECT_NEAR(n.norm(), 1.0, 1e-9);

    Vec2 zero = (Vec2{0.0, 0.0}).normalized();
    EXPECT_NEAR(zero.x, 0.0, 1e-9);
    EXPECT_NEAR(zero.y, 0.0, 1e-9);
}

TEST(Vec2, ClosestOnSegment) {
  Vec2 a{0.0, 0.0}, b{2.0, 0.0};

  Closest c1 = closest_on_segment(Vec2{1.0, 1.0}, a, b);
  EXPECT_NEAR(c1.point.x, 1.0, 1e-9);
  EXPECT_NEAR(c1.point.y, 0.0, 1e-9);
  EXPECT_NEAR(c1.dist, 1.0, 1e-9);

  Closest c2 = closest_on_segment(Vec2{3.0, 1.0}, a, b);
  EXPECT_NEAR(c2.point.x, 2.0, 1e-9);
  EXPECT_NEAR(c2.dist, std::sqrt(2.0) , 1e-9);

  Closest c3 = closest_on_segment(Vec2{-1.0, 0.0}, a, b);
  EXPECT_NEAR(c3.point.x, 0.0, 1e-9);
  EXPECT_NEAR(c3.dist, 1.0, 1e-9);
}