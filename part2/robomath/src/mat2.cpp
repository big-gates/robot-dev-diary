#include <cmath>
#include <robomath/mat2.hpp>

namespace robomath {

Mat2 rotation(double theta) {
  return Mat2{std::cos(theta), -std::sin(theta), 
              std::sin(theta),std::cos(theta)};
}

Mat2 Mat2::operator*(const Mat2& m) const {
    return Mat2{a * m.a + b * m.c, a * m.b + b * m.d,
                 c * m.a + d * m.c,  c * m.b + d * m.d};
}

Vec2 Mat2::operator*(const Vec2& v) const {
    return Vec2{a * v.x + b * v.y, c * v.x + d * v.y};
}

Mat2 Mat2::transpose() const {
    return Mat2 {
        a, c,
        b, d
    };
}

} // namespace robomath
