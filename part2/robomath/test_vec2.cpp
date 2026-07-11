#include "angles.hpp"
#include "vec2.hpp"
#include <gtest/gtest.h>

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