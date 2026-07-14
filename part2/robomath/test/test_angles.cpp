#include <robomath/angles.hpp>

#include <gtest/gtest.h>
#include <vector>

using namespace robomath;

TEST(Angles, Deg2Rad) {
  EXPECT_NEAR(deg2rad(180.0), kPi, 1e-9);
  EXPECT_NEAR(deg2rad(90.0), kPi / 2.0, 1e-9);
  EXPECT_NEAR(deg2rad(0.0), 0.0, 1e-9);
}

TEST(Angles, WrapToPi) {
  EXPECT_NEAR(wrap_to_pi(deg2rad(360.0)), 0.0, 1e-9);
  EXPECT_NEAR(wrap_to_pi(deg2rad(-190.0)), deg2rad(170.0), 1e-9);
  EXPECT_NEAR(wrap_to_pi(deg2rad(540.0)), kPi, 1e-9);
  EXPECT_NEAR(wrap_to_pi(deg2rad(90.0)), deg2rad(90.0), 1e-9);
}

TEST(Angles, ShortestTurn) {
  EXPECT_NEAR(shortest_turn(deg2rad(350.0), deg2rad(10.0)), deg2rad(20.0),
              1e-9); //+20도
  EXPECT_NEAR(shortest_turn(deg2rad(10.0), deg2rad(350.0)), deg2rad(-20.0),
              1e-9);                                                   //-20도
  EXPECT_NEAR(shortest_turn(deg2rad(0.0), deg2rad(180.0)), kPi, 1e-9); //+180도
}

TEST(Angles, BearingTo) {
  EXPECT_NEAR(bearing_to(0, 0, 1, 1), deg2rad(45.0), 1e-9);
  EXPECT_NEAR(bearing_to(0, 0, -1, 0), deg2rad(180.0), 1e-9);
}

TEST(Angles, Relativebearing) {
  EXPECT_NEAR(relative_bearing(2, 3, deg2rad(90.0), 2, 5), 0.0, 1e-9);
  EXPECT_NEAR(relative_bearing(0, 0, deg2rad(0.0), 0, -3), deg2rad(-90.0),
              1e-9);
}

TEST(Angles, IntegrateHeading) {
  std::vector<double> omega(10, 0.1);

  EXPECT_NEAR(integrate_heading(0.0, omega, 0.1), 0.1, 1e-9);
  EXPECT_NEAR(integrate_heading(0.5, {}, 0.1), 0.5, 1e-9);
}

TEST(Anlges, InFov) {
      double fov = deg2rad(60.0);   // 전체 60도 시야각 → 정면 기준 ±30도                                                                                                                                          
                                                                                                                                                                                                                 
    // 로봇 (0,0)에서 동쪽(0도)을 봄                                                                                                                                                                             
    EXPECT_TRUE(in_fov(0, 0, 0.0, 2.0, 0.5, fov));   // 상대 베어링 ≈ 14도 → 안에 있음                                                                                                                           
    EXPECT_FALSE(in_fov(0, 0, 0.0, 2.0, 2.0, fov));  // ≈ 45도 > 30도 → 벗어남                                                                                                                                   
    EXPECT_FALSE(in_fov(0, 0, 0.0, -1.0, 0.0, fov)); // 등 뒤(180도) → 당연히 벗어남
}