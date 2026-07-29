#include "robomath/vec2.hpp"
#include <cmath>
#include <optional>
#include <robomath/line.hpp>
#include <vector>

namespace robomath {

std::optional<Line> fit_line(const std::vector<Vec2> &pts) {
  if (pts.size() < 2) {
    return std::nullopt;
  }

  double sigma_xy = 0.0;
  double sigma_x = 0.0;
  double sigma_y = 0.0;
  double sigma_square_x = 0.0;
  int n = pts.size();
  for (Vec2 v : pts) {
    sigma_xy += v.x * v.y;
    sigma_x += v.x;
    sigma_y += v.y;
    sigma_square_x += std::pow(v.x, 2);
  }

  double slope = (n * sigma_xy - sigma_x * sigma_y) /
                 (n * sigma_square_x - std::pow(sigma_x, 2));
  double intercept = (sigma_y - slope * sigma_x) / n;

  return Line{slope, intercept};
}

std::optional<Vec2> intersect(const Line &l1, const Line &l2) {
  if (std::abs(l1.slope - l2.slope) < 1e-9) {
    return std::nullopt;
  }

  double x = (l2.intercept - l1.intercept) / (l1.slope - l2.slope);
  double y = l1.slope * x + l1.intercept;
  return Vec2{x, y};
}

} // namespace robomath