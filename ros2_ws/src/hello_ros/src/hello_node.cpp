#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include <chrono>

using namespace std::chrono_literals;

class HelloNode : public rclcpp::Node {
public:
  HelloNode() : Node("hello_node") {
    publisher_ = create_publisher<std_msgs::msg::String>("hello", 10);
    timer_ = create_wall_timer(1s, [this] {
      std_msgs::msg::String msg;
      msg.data = "robot-dev-diary chapter A, count " + std::to_string(count_++);
      RCLCPP_INFO(get_logger(), "publishing: '%s'", msg.data.c_str());
      publisher_->publish(msg);
    });
  }

private:
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
  int count_{0};
};

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<HelloNode>());
  rclcpp::shutdown();
  return 0;
}