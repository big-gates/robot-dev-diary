
#include <cmath>
#include <robomath/mat3.hpp>

namespace robomath {

Vec2 Mat3::operator*(const Vec2& v) const {
  return Vec2{
    a * v.x + b * v.y + c, 
    d * v.x + e * v.y + f
  };
}

Mat3 Mat3::operator*(const Mat3& m) const {
    return Mat3{
        a * m.a + b * m.d,
        a * m.b + b * m.e,
        a * m.c + b * m.f + c,

        d * m.a + e * m.d,
        d * m.b + e * m.e,
        d * m.c + e * m.f + f
    };
}

Mat3 transform(double tx, double ty, double theta){
    return Mat3{
        std::cos(theta), -std::sin(theta), tx,
        std::sin(theta), std::cos(theta), ty
    };
}

} // namespace robomath