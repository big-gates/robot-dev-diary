#pragma once
#include "renderer.hpp" // Pose2D를 renderer와 공유 (타입 통일)

// 계약서: "로봇의 몸이라면 이건 할 줄 알아야 한다"
class RobotHAL {
public:
  virtual ~RobotHAL() = default;
  virtual void set_wheel_speeds(double left, double right) = 0;
  virtual Pose2D read_pose() const = 0;
};

// 계약 이행 #1: 시뮬레이터 몸
class SimRobot : public RobotHAL {
public:
  // track: 좌우 바퀴 간격(m), start: 로봇이 놓인 초기 포즈
  SimRobot(double track, const Pose2D &start) : track_(track), pose_(start) {}

  void set_wheel_speeds(double left, double right) override;

  // dt초 동안 현재 바퀴 속도로 움직였다고 가정하고 포즈 적분
  void step(double dt);

  Pose2D read_pose() const override;

private:
  double track_;
  double left_ = 0.0, right_ = 0.0;
  Pose2D pose_;
};