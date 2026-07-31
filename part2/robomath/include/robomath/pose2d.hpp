#include "robomath/vec2.hpp"

namespace robomath {
struct Pose2D {
  double x{}, y{}, theta{};
  Vec2 transform(Vec2 local) const;
  Pose2D operator*(const Pose2D &) const;
};

Vec2 inverse_transform(const Pose2D &robot, Vec2 map_point);
Pose2D inverse(const Pose2D &);
}; // namespace robomath