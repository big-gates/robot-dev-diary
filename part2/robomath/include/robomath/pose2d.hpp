#include "robomath/vec2.hpp"

namespace robomath {
struct Pose2D {
  double x{}, y{}, theta{};
  Vec2 transform(Vec2 local) const;
  Pose2D operator*(const Pose2D &) const;
};

Vec2 inverse_transform(const Pose2D &robot, Vec2 map_point);

Pose2D inverse(const Pose2D &);

Vec2 sensor_to_map(const Pose2D &robot, const Pose2D &mount, Vec2 sensed);

Pose2D approach_pose(const Pose2D &target, double standoff);

Pose2D integrate_odom(const Pose2D &p, double dl, double dr, double track);
} // namespace robomath