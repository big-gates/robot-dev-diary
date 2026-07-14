#include "robomath/angles.hpp"
#include <robomath/mat3.hpp>
#include <gtest/gtest.h>

using namespace robomath;

TEST(Mat3, RotateThenTranslate) {
    Vec2 r = transform(1.0, 2.0, deg2rad(90.0)) * Vec2{1.0, 0.0};
    EXPECT_NEAR(r.x, 1.0, 1e-9);
    EXPECT_NEAR(r.y, 3.0, 1e-9);
}

TEST(Mat3, Identity){
    Vec2 r = transform(0.0, 0.0, 0.0) * Vec2{5.0 , 7.0};
    EXPECT_NEAR(r.x, 5.0, 1e-9);
    EXPECT_NEAR(r.y, 7.0, 1e-9);
}

TEST(Mat3, Composition) {
    // 로봇 -> 지도: 로봇이 (3,0), 방향 90도 
    Mat3 map_from_robot = transform(3.0, 0.0, deg2rad(90.0));
    // 센서 -> 로봇: 센서가 로봇에 1m 에 붙어있다 (회전 없음)
    Mat3 robot_from_sensor = transform(1.0, 0.0, 0.0);

    // 센서 -> 지도
    Mat3 map_from_sensor = map_from_robot * robot_from_sensor;

    //센서 좌표계의 원점(0,0)은 지도에서 어디인가?
    Vec2 p = map_from_sensor * Vec2{0.0, 0.0};

    EXPECT_NEAR(p.x, 3.0, 1e-9);
    EXPECT_NEAR(p.y, 1.0, 1e-9);
}