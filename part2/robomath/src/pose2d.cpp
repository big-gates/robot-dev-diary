#include "robomath/pose2d.hpp"
#include "robomath/angles.hpp"
#include "robomath/vec2.hpp"
#include <cmath>

namespace robomath {
Vec2 Pose2D::transform(Vec2 local) const {
  return {(std::cos(theta) * local.x - std::sin(theta) * local.y) + x,
          (std::sin(theta) * local.x + std::cos(theta) * local.y) + y};
}

Pose2D Pose2D::operator*(const Pose2D & other) const { 
    Vec2 pos = this->transform({other.x, other.y});
    double th = wrap_to_pi(theta + other.theta);
    return Pose2D{pos.x, pos.y, th}; 
}
} // namespace robomath