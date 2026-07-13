#include "renderer.hpp"

#include "angles.hpp"
#include "sim_robot.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>

int main() {
  ViewConfig view; // 40 x 20 칸, 0.25m/칸 → 10m x 5m 공간

  const double goal_x = 7.0, goal_y = 3.5;
  const double dt = 0.1;
  const double kArriveRadius = 0.02;
  const double kTrack = 0.5;
  const double kSpeedGain = 0.8;
  const double kTurnGain = 1.5;
  const double kMaxSpeed = 0.6;
  const double kMaxOmega = 2.0;

  Pose2D start{1.0, 1.0, 0.0};
  SimRobot sim(0.5, start);
  bool arrived = false;

  for (int i = 0; i < 500; ++i) {
    Pose2D p = sim.read_pose();

    double bearing = relative_bearing(p.x, p.y, p.theta, goal_x, goal_y);
    double dist = std::hypot(goal_x - p.x, goal_y - p.y);

    if (dist < kArriveRadius) {
      arrived = true;
      break;
    }

    double omega = kTurnGain * bearing;
    double v = kSpeedGain * dist * std::cos(bearing);
    if (v < 0)
      v = 0.0;

    v = std::clamp(v, 0.0, kMaxSpeed);
    omega = std::clamp(omega, -kMaxOmega, kMaxOmega);

    sim.set_wheel_speeds(v - omega * kTrack / 2.0, v + omega * kTrack / 2.0);

    sim.step(dt);

    std::cout << ansi::kClear;
    std::cout << render(sim.read_pose(), goal_x, goal_y, view);
    std::cout.flush();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  if (arrived) {
    std::cout << "\n🐢 도착!\n";
  } else {
    std::cout << "\n❌ 실패: 500스텝 내에 도착하지 못함\n";
  }

  return 0;
}
