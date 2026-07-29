#pragma once
#include <optional>
#include <robomath/vec2.hpp>
#include <vector>

namespace robomath {
struct Line {
  double slope;     // 기울기 (a)
  double intercept; // y절편 (b) -> y = slope * x + intercept
};

std::optional<Line> fit_line(const std::vector<Vec2> &pts);
std::optional<Vec2> intersect(const Line &l1, const Line &l2);
} // namespace robomath