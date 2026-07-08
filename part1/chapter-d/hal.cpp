#include <cassert>
#include <cmath>
#include <iostream>

struct Pose {
  double x = 0.0, y = 0.0, theta = 0.0;
};

// 계약서: "로봇의 몸이라면 이건 할 줄 알아야 한다"
class RobotHAL {
public:
  virtual ~RobotHAL() = default;
  virtual void set_wheel_speeds(double left, double right) = 0;
  virtual Pose read_pose() const = 0;
};

// 계약 이행 #1: 시뮬레이터 몸
class SimRobot : public RobotHAL {
public:
  SimRobot(double track) : track_(track) {}

  void set_wheel_speeds(double left, double right) override {
    left_ = left;
    right_ = right;
  }

  // dt초 동안 현재 바퀴 속도로 움직였다고 가정하고 포즈 적분
  void step(double dt) {
    double v = (left_ + right_) / 2.0; // 전진 속도 = 두 바퀴 평균
    double omega = (right_ - left_) / track_; // 회전 속도 = 속도차 / 바퀴간격
    pose_.x += v * std::cos(pose_.theta) * dt;
    pose_.y += v * std::sin(pose_.theta) * dt;
    pose_.theta += omega * dt;
  }

  Pose read_pose() const override { return pose_; }

private:
  double track_;
  double left_ = 0.0, right_ = 0.0;
  Pose pose_;
};

// 핵심: 브레인은 RobotHAL만 안다. SimRobot을 모른다
void drive_forward(RobotHAL &robot) {
  robot.set_wheel_speeds(1.0, 1.0);
}

int main() {
  SimRobot sim(0.5);

  drive_forward(sim); // 브레인 호출
  sim.step(1.0);           // 시뮬 시간 1초 진행

  Pose p = sim.read_pose();
  assert(std::abs(p.x - 1.0) < 1e-9); // 1m/s로 1초 직진 -> x = 1
  assert(std::abs(p.y - 0.0) < 1e-9);
  assert(std::abs(p.theta - 0.0) < 1e-9);

  std::cout << "hal ok, pose.x=" << p.x << "\n";

  SimRobot curve(0.5);
  curve.set_wheel_speeds(1.0, 0.5); // 오른쪽이 느림 -> 오른쪽으로 휜다
  std::cout << "\n곡선 주행 (step) 0.1s씩 10번:\n";
  for (int i = 0; i < 10; ++i){
    curve.step(0.1);
    Pose c = curve.read_pose();
    std::cout << " t=" << (i + 1) * 0.1 << "s x=" << c.x << " y=" << c.y << " theta=" << c.theta << "\n";
  }
}