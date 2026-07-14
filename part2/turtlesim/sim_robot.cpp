#include "sim_robot.hpp"
#include <robomath/angles.hpp>

using namespace robomath;
#include <cmath>

void SimRobot::set_wheel_speeds(double left, double right) {
  left_ = left;
  right_ = right;
}

Pose2D SimRobot::read_pose() const { return pose_; }

void SimRobot::step(double dt) {
    double v = (left_ + right_) / 2.0;
    double omega = (right_ - left_) / track_;
    pose_.x += v * std::cos(pose_.theta) * dt;
    pose_.y += v * std::sin(pose_.theta) * dt;
    pose_.theta = wrap_to_pi(pose_.theta + omega * dt);
}