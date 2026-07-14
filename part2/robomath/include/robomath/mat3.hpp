#pragma once
#include <robomath/vec2.hpp>

namespace robomath {

struct Mat3 {
  double a = 1.0, b = 0.0, c = 0.0;
  double d = 0.0, e = 1.0, f = 0.0;

  Vec2 operator*(const Vec2 &v) const;
  Mat3 operator*(const Mat3 &m) const;
};

Mat3 transform(double tx, double ty, double theta);

} // namespace robomath