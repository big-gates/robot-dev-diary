#include "robomath/angles.hpp"
#include "robomath/pose2d.hpp"
#include "robomath/vec2.hpp"
#include <gtest/gtest.h>

using namespace robomath;

TEST(Pose2D, Transform) {
    Pose2D pose = Pose2D{1, 1,deg2rad(90.0)};
    Vec2 result = pose.transform({1,0});
    EXPECT_NEAR(result.x, 1.0, 1e-9);
    EXPECT_NEAR(result.y, 2.0, 1e-9);
}

TEST(Pose2D, Compose) {
    Pose2D pose = Pose2D{1, 0,deg2rad(90.0)};
    Pose2D other = Pose2D{1, 0,deg2rad(0.0)};
    Pose2D result = pose * other;
    EXPECT_NEAR(result.x, 1.0, 1e-9);
    EXPECT_NEAR(result.y, 1.0, 1e-9);
    EXPECT_NEAR(result.theta, deg2rad(90.0), 1e-9);
}

TEST(Pose2D, Identity) {
    Pose2D p = Pose2D{0, 0, 0.0};
    Pose2D pose = Pose2D{1, 0,deg2rad(90.0)};
    Pose2D result = p * pose;
    EXPECT_NEAR(result.x, 1.0, 1e-9);
    EXPECT_NEAR(result.y, 0.0, 1e-9);
    EXPECT_NEAR(result.theta, deg2rad(90.0), 1e-9);
}