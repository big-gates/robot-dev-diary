#include "angles.hpp"
#include <gtest/gtest.h>

TEST(Angles, Deg2Rad) {
  EXPECT_NEAR(deg2rad(180.0), kPi, 1e-9);
  EXPECT_NEAR(deg2rad(90.0), kPi / 2.0, 1e-9);
  EXPECT_NEAR(deg2rad(0.0), 0.0, 1e-9);
}