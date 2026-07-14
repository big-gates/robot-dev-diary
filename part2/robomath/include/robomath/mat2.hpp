#pragma once
#include <robomath/vec2.hpp>

namespace robomath {

struct Mat2 {
  double a = 0.0, b = 0.0, c = 0.0, d = 0.0;

  Vec2 operator*(const Vec2 &v) const; // 행렬 x 벡터 = 벡터 (회전 적용)
  Mat2 operator*(const Mat2 &m) const; // 행렬 x 행렬 = 행렬 (회전 합성)
  Mat2 transpose() const;
};

Mat2 rotation(double theta);

} // namespace robomath