#include "robomath/vec2.hpp"
#include <vector>

namespace robomath {
struct GridSpec {
  double origin_x;
  double origin_y;
  double resolution;
};

struct Cell {
  int cx;
  int cy;

  bool operator==(const Cell &o) const { return cx == o.cx && cy == o.cy; }
};

Cell world_to_grid(const GridSpec &, Vec2 world);
Vec2 grid_to_world(const GridSpec &, Cell);
std::vector<Cell> raycast(Cell from, Cell to);

} // namespace robomath