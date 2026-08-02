#include <gtest/gtest.h>
#include <robomath/grid.hpp>

using namespace robomath;

// origin(-5,-5), res 0.05, world(0,0) → cell(100,100)
TEST(Grid, WorldToGrid) {
  GridSpec spec = GridSpec{-5, -5, 0.05};
  Vec2 world = Vec2{0, 0};
  Cell result = world_to_grid(spec, world);

  EXPECT_EQ(result.cx, 100);
  EXPECT_EQ(result.cy, 100);
}

// grid_to_world(world_to_grid(p)) 오차 ≤ res/2
TEST(Grid, RoundTrip) {
  GridSpec spec{-5.0, -5.0, 0.05};
  Vec2 p{2.37, 1.08}; // 아무 점 (칸 경계는 피해서)
  Cell c = world_to_grid(spec, p);
  Vec2 back = grid_to_world(spec, c);
  EXPECT_NEAR(back.x, p.x, spec.resolution / 2); // 반 칸 이내로 복귀
  EXPECT_NEAR(back.y, p.y, spec.resolution / 2);
}

TEST(Grid, WorldToGridNegative) {
  GridSpec spec{-5.0, -5.0, 0.05};
  Cell c = world_to_grid(spec, {-5.03, -5.03});

  EXPECT_EQ(c.cx, -1); // floor 없으면 0이 나와서 실패 → floor 검증됨
  EXPECT_EQ(c.cy, -1);
}

// (0,0)→(3,1) → 4칸, 시작·끝 포함, 8-이웃
TEST(Grid, RaycastLine) {
  std::vector<Cell> result = raycast({0, 0}, {3, 1});
  std::vector<Cell> expected = {{0, 0}, {1, 0}, {2, 1}, {3, 1}};
  EXPECT_EQ(result, expected);
}

// 같은 칸이면 1개만 나오는지도 확인 (steps=0 처리 검증):
TEST(Grid, RaycastSameCell) {
  std::vector<Cell> result = raycast({5, 5}, {5, 5});
  std::vector<Cell> expected = {{5, 5}};
  EXPECT_EQ(result, expected);
}