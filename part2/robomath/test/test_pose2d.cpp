#include "robomath/angles.hpp"
#include "robomath/pose2d.hpp"
#include "robomath/vec2.hpp"
#include <gtest/gtest.h>

using namespace robomath;

// 3-1 transform: 로봇 로컬 좌표의 점을 지도 좌표로.
// 로봇 (1,1)에서 90°(북쪽) 봄, 정면 1m 앞 점 (1,0)local.
// 90° 회전 → (1,0)이 (0,1)로, 거기에 (1,1) 이동 → (1,2).
TEST(Pose2D, Transform) {
  Pose2D pose = Pose2D{1, 1, deg2rad(90.0)};
  Vec2 result = pose.transform({1, 0});
  EXPECT_NEAR(result.x, 1.0, 1e-9);
  EXPECT_NEAR(result.y, 2.0, 1e-9);
}

// 3-1 operator*: 변환 합성 (지도←로봇 ∘ 로봇←센서 = 지도←센서).
// A{1,0,90°} ∘ B{1,0,0°}: 위치 = A.transform((1,0)) = (1,1), 각도 = 90°+0°.
TEST(Pose2D, Compose) {
  Pose2D pose = Pose2D{1, 0, deg2rad(90.0)};
  Pose2D other = Pose2D{1, 0, deg2rad(0.0)};
  Pose2D result = pose * other;
  EXPECT_NEAR(result.x, 1.0, 1e-9);
  EXPECT_NEAR(result.y, 1.0, 1e-9);
  EXPECT_NEAR(result.theta, deg2rad(90.0), 1e-9);
}

// 3-1 항등원: Pose(0,0,0°) ∘ P = P. 아무것도 안 하는 변환을 곱하면 그대로.
TEST(Pose2D, Identity) {
  Pose2D p = Pose2D{0, 0, 0.0};
  Pose2D pose = Pose2D{1, 0, deg2rad(90.0)};
  Pose2D result = p * pose;
  EXPECT_NEAR(result.x, 1.0, 1e-9);
  EXPECT_NEAR(result.y, 0.0, 1e-9);
  EXPECT_NEAR(result.theta, deg2rad(90.0), 1e-9);
}

// 3-2 inverse_transform: 지도 좌표의 점을 로봇 로컬 좌표로 (transform의 역방향).
// 로봇 (1,1,90°), 지도점 (1,2)는 로봇 정면 1m 북쪽 → 로봇 기준 (1,0).
TEST(Pose2D, InverseTransform) {
  Pose2D robot = Pose2D{1, 1, deg2rad(90.0)};
  Vec2 map_point = Vec2{1, 2};
  Vec2 result = inverse_transform(robot, map_point);
  EXPECT_NEAR(result.x, 1.0, 1e-9);
  EXPECT_NEAR(result.y, 0.0, 1e-9);
}

// 3-2 inverse: 속성 기반 테스트. inverse(P) * P = 항등원(0,0,0).
// "되돌린 뒤 다시 하면 제자리" — inverse와 operator*를 한 번에 검증.
TEST(Pose2D, Inverse) {
  Pose2D p = Pose2D{2, 3, deg2rad(45.0)};
  Pose2D result = inverse(p) * p;
  EXPECT_NEAR(result.x, 0.0, 1e-9);
  EXPECT_NEAR(result.y, 0.0, 1e-9);
  EXPECT_NEAR(result.theta, 0.0, 1e-9);
}

// 3-2 왼쪽/오른쪽 판정: 로봇 로컬에서 +y=왼쪽, -y=오른쪽.
// 로봇 (0,0,0°)(동쪽 봄) 기준: (1,1)=북동=왼쪽(y>0), (1,-1)=남동=오른쪽(y<0).
// 양쪽 다 검증해서 "항상 같은 부호만 반환" 버그까지 차단.
TEST(Pose2D, LeftRight) {
  Pose2D robot = Pose2D{0, 0, 0.0};

  Vec2 left = inverse_transform(robot, {1, 1});
  EXPECT_GT(left.y, 0.0);

  Vec2 right = inverse_transform(robot, {1, -1});
  EXPECT_LT(right.y, 0.0);
}

// 3-3 sensor_to_map: 센서→로봇→지도 2단 변환 체인 (= TF2가 하는 일).
// 로봇 (0,0,90°)에 mount(정면 0.15m) 달린 센서가 잰 점 (1,0)sensor.
// robot*mount 합성 후 sensed 변환 → 지도 (0, 1.15).
TEST(Pose2D, SensorToMap) {
    Pose2D robot = Pose2D {0, 0, deg2rad(90.0)};
    Pose2D mount = Pose2D { 0.15, 0, 0};
    Vec2 sensed = Vec2 {1, 0};
    Vec2 result = sensor_to_map(robot, mount, sensed);

    EXPECT_NEAR(result.x, 0.0, 1e-9);
    EXPECT_NEAR(result.y, 1.15, 1e-9);
}