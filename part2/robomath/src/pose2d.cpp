#include "robomath/pose2d.hpp"
#include "robomath/angles.hpp"
#include "robomath/vec2.hpp"
#include <cmath>

namespace robomath {
Vec2 Pose2D::transform(Vec2 local) const {
  return {(std::cos(theta) * local.x - std::sin(theta) * local.y) + x,
          (std::sin(theta) * local.x + std::cos(theta) * local.y) + y};
}

Pose2D Pose2D::operator*(const Pose2D &other) const {
  Vec2 pos = this->transform({other.x, other.y});
  double th = wrap_to_pi(theta + other.theta);
  return Pose2D{pos.x, pos.y, th};
}

Vec2 inverse_transform(const Pose2D &robot, Vec2 map_point) {
  double dx = map_point.x - robot.x;
  double dy = map_point.y - robot.y;
  return Vec2{dx * std::cos(robot.theta) + dy * std::sin(robot.theta),
              dx * -std::sin(robot.theta) + dy * std::cos(robot.theta)};
}

Pose2D inverse(const Pose2D &p) {
  Vec2 v = inverse_transform(p, {0, 0});
  double th = wrap_to_pi(-p.theta);
  return Pose2D{v.x, v.y, th};
}

Vec2 sensor_to_map(const Pose2D &robot, const Pose2D &mount, Vec2 sensed) {
  return (robot * mount).transform(sensed);
}

Pose2D approach_pose(const Pose2D &target, double standoff) {
    Vec2 t = target.transform({standoff, 0});
    double heading = wrap_to_pi(target.theta + kPi);
    return Pose2D {t.x, t.y, heading};
}
} // namespace robomath