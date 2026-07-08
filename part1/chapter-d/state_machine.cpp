#include <cassert>
#include <iostream>
enum class RobotState { IDLE, MOVING, ARRIVED, RETURNING };

class ServingRobot {
public:
  RobotState state() const { return state_; }

  bool transition_to(RobotState next) {
    if (next == next_of(state_)) {
      state_ = next;
      return true;
    } else {
      return false;
    }
  }

private:
  RobotState state_ = RobotState::IDLE;

  static RobotState next_of(RobotState robot_state) {
    switch (robot_state) {

    case RobotState::IDLE:
      return RobotState::MOVING;
    case RobotState::MOVING:
      return RobotState::ARRIVED;
    case RobotState::ARRIVED:
      return RobotState::RETURNING;
    case RobotState::RETURNING:
      return RobotState::IDLE;
    }
    return robot_state;
  }
};

int main() {
  ServingRobot r;
  assert(r.state() == RobotState::IDLE);
  assert(r.transition_to(RobotState::ARRIVED) == false); // 순간이동 금지
  assert(r.state() == RobotState::IDLE);                 // 상태 불변
  assert(r.transition_to(RobotState::MOVING) == true);
  assert(r.transition_to(RobotState::MOVING) == false); // 자기 자신 금지
  assert(r.transition_to(RobotState::ARRIVED) == true);
  assert(r.transition_to(RobotState::RETURNING) == true);
  assert(r.transition_to(RobotState::IDLE));
  std::cout << "state machine ok\n";
}