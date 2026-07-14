#include <gtest/gtest.h>
#include <robomath/mat2.hpp>
#include <robomath/angles.hpp>

using namespace robomath;

TEST(Mat2, RotateVector) {
    Vec2 r = rotation(deg2rad(90.0)) * Vec2{1.0, 0.0};
    EXPECT_NEAR(r.x, 0.0, 1e-9);
    EXPECT_NEAR(r.y, 1.0, 1e-9);
}

TEST(Mat2, Composition) {
    Mat2 m = rotation(deg2rad(30.0)) * rotation(deg2rad(60.0));
    Mat2 expect = rotation(deg2rad(90.0));
    EXPECT_NEAR(m.a, expect.a, 1e-9);
    EXPECT_NEAR(m.b, expect.b, 1e-9);
    EXPECT_NEAR(m.c, expect.c, 1e-9);
    EXPECT_NEAR(m.d, expect.d, 1e-9);
}

TEST(Mat2, TransposeIsInverse) {
    Mat2 r = rotation(deg2rad(37.0));
    Mat2 identity = r.transpose() * r;
    EXPECT_NEAR(identity.a, 1.0, 1e-9);
    EXPECT_NEAR(identity.b, 0.0, 1e-9);
    EXPECT_NEAR(identity.c, 0.0, 1e-9);
    EXPECT_NEAR(identity.d, 1.0, 1e-9);
}