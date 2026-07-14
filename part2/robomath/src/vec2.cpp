#include <algorithm>
#include <robomath/vec2.hpp>

#include <cmath>

namespace robomath {

Vec2 Vec2::operator+(const Vec2 &o) const { return Vec2{x + o.x, y + o.y}; }

Vec2 Vec2::operator-(const Vec2 &o) const { return Vec2{x - o.x, y - o.y}; }

Vec2 Vec2::operator*(double s) const { return Vec2{x * s, y * s}; }

double Vec2::dot(const Vec2 &o) const { return x * o.x + y * o.y; }

double Vec2::norm() const { return std::hypot(x, y); }

Vec2 Vec2::normalized() const {
  double n = norm();
  if (n == 0.0) { // 정책: 영벡터는 방향이 없으므로 (0,0)
    return Vec2{0.0, 0.0};
  }
  return Vec2{x / n, y / n};
}

Vec2 project_forward(double x, double y, double theta, double dist) {
  // cos = x몫, sin = y몫 — 전진 거리를 헤딩 방향으로 분해
  return Vec2{x + dist * std::cos(theta), y + dist * std::sin(theta)};
}

Closest closest_on_segment(Vec2 p, Vec2 a, Vec2 b) {
  Vec2 ab = b - a;
  Vec2 ap = p - a;

  double t = ap.dot(ab) / ab.dot(ab);

  t = std::clamp(t, 0.0, 1.0);

  Vec2 closest = a + ab * t;
  double dist = (p - closest).norm();

  return Closest{
    closest,
    dist,
  };
}
} // namespace robomath
