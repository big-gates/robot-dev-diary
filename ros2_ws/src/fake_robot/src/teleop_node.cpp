#include <termios.h>
#include <unistd.h>

#include <geometry_msgs/msg/twist.hpp>
#include <rclcpp/rclcpp.hpp>

class TeleopNode : public rclcpp::Node {
public:
  TeleopNode() : rclcpp::Node("teleop_node") {
    RCLCPP_INFO(get_logger(), "teleop started");

    publisher_ = create_publisher<geometry_msgs::msg::Twist>("cmd_vel", 10);
    timer_ = create_wall_timer(timer_period_, [this]() { on_timer(); });
  }

private:
  double linear_{0.0};
  double angular_{0.0};

  const std::chrono::milliseconds timer_period_ = std::chrono::milliseconds(50);
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;

  void on_timer() {
    char c;
    if (read(STDIN_FILENO, &c, 1) > 0) {
      switch (c) {
      case 'w':
        linear_ = 0.3;
        break;
      case 's':
        linear_ = -0.3;
        break;
      case 'a':
        angular_ = 0.5;
        break;
      case 'd':
        angular_ = -0.5;
        break;
      case 'x':
        linear_ = 0.0;
        angular_ = 0.0;
        break;
      case 'q':
        rclcpp::shutdown();
        break;
      default:
        break;
      }
    }
    geometry_msgs::msg::Twist msg;
    msg.linear.x = linear_;
    msg.angular.z = angular_;
    publisher_->publish(msg);
  }
};

class RawTerminal {
public:
  RawTerminal() {
    tcgetattr(STDIN_FILENO, &original_);

    termios raw = original_;         // 복사본을 수정
    raw.c_lflag &= ~(ICANON | ECHO); // 줄단위 입력 / 에코 끄기
    raw.c_cc[VMIN] = 0;              // 0글자여도 반환
    raw.c_cc[VTIME] = 0;             // 대기 안 함
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
  }
  ~RawTerminal() { tcsetattr(STDIN_FILENO, TCSANOW, &original_); }

  RawTerminal(const RawTerminal &) = delete;
  RawTerminal &operator=(const RawTerminal &) = delete;

private:
  termios original_;
};

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);
  RawTerminal terminal;
  auto node = std::make_shared<TeleopNode>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}