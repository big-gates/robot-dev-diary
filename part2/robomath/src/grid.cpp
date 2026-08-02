#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <robomath/grid.hpp>
#include <vector>

namespace robomath {
Cell world_to_grid(const GridSpec &spec, Vec2 world) {
  int cx = std::floor((world.x - spec.origin_x) / spec.resolution);
  int cy = std::floor((world.y - spec.origin_y) / spec.resolution);

  return Cell{cx, cy};
}

Vec2 grid_to_world(const GridSpec &spec, Cell cell) {
  double world_x = spec.origin_x + (cell.cx + 0.5) * spec.resolution;
  double world_y = spec.origin_y + (cell.cy + 0.5) * spec.resolution;

  return Vec2{world_x, world_y};
}

std::vector<Cell> raycast(Cell from, Cell to) {
  int dx = to.cx - from.cx;
  int dy = to.cy - from.cy;
  int steps = std::max(abs(dx), abs(dy));

  if (steps == 0) {
    return {from}; // 같은 칸 → 점 하나, 0으로 나누기 회피
  }
  
  std::vector<Cell> cells;
  for (int step = 0; step <= steps; ++step) {
    double t = static_cast<double>(step) / steps;
    int x = std::round(from.cx + t * dx);
    int y = std::round(from.cy + t * dy);

    cells.push_back({.cx = x, .cy = y});
  }

  return cells;
}
} // namespace robomath