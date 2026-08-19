#include <rclcpp/rclcpp.hpp>
#include <robomath/pose2d.hpp>
#include <sensor_msgs/msg/joint_state.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <tf2/LinearMath/Quaternion.hpp>

class OdomNode : public rclcpp::Node {
public:
  OdomNode() : rclcpp::Node("odom_node") {
    RCLCPP_INFO(get_logger(), "odom_node started");

    sub_ = create_subscription<sensor_msgs::msg::JointState>(
        "joint_states", 10,
        [this](const sensor_msgs::msg::JointState::SharedPtr msg) { on_joint_state(msg); });

    pub_ = create_publisher<nav_msgs::msg::Odometry>("odom_mine", 10);
  }

private:
  robomath::Pose2D pose_{};
  double prev_left_{};
  double prev_right_{};
  bool have_prev_{false};

  const double wheel_radius_{0.08};
  const double track_{0.4};

  rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr sub_;
  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr pub_;

  void on_joint_state(const sensor_msgs::msg::JointState::SharedPtr msg) {
    double left_w{0.0};
    double right_w{0.0};
    for (size_t i = 0; i < msg->name.size(); ++i) {
      if (msg->name[i] == "left_wheel_joint") {
        left_w = msg->position[i];
      } else if (msg->name[i] == "right_wheel_joint") {
        right_w = msg->position[i];
      }
    }

    if (!have_prev_) {
      prev_left_ = left_w;
      prev_right_ = right_w;
      have_prev_ = true;
      return;
    }

    double dl = (left_w - prev_left_) * wheel_radius_;
    double dr = (right_w - prev_right_) * wheel_radius_;
    pose_ = robomath::integrate_odom(pose_, dl, dr, track_);
    prev_left_ = left_w;
    prev_right_ = right_w;

    nav_msgs::msg::Odometry odom_msg;
    odom_msg.header.stamp = now();
    odom_msg.header.frame_id = "odom";
    odom_msg.child_frame_id = "base_link";
    odom_msg.pose.pose.position.x = pose_.x;
    odom_msg.pose.pose.position.y = pose_.y;

    const double half_theta = pose_.theta / 2.0;

    odom_msg.pose.pose.orientation.x = 0.0;
    odom_msg.pose.pose.orientation.y = 0.0;
    odom_msg.pose.pose.orientation.z = std::sin(half_theta);
    odom_msg.pose.pose.orientation.w = std::cos(half_theta);

    pub_->publish(odom_msg);

    RCLCPP_INFO_THROTTLE(get_logger(), *get_clock(), 1000, "mine: x=%.4f y=%.4f th=%.4f", pose_.x,
                         pose_.y, pose_.theta);
  }
};

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);
  auto node = std::make_shared<OdomNode>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}