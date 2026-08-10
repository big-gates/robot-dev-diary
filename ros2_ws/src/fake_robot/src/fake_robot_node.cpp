#include <chrono>
#include <cmath>
#include <memory>
#include <mutex>

#include <geometry_msgs/msg/twist.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <rclcpp/callback_group.hpp>
#include <rclcpp/executors/multi_threaded_executor.hpp>
#include <rclcpp/qos.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp/subscription_options.hpp>
#include <robomath/pose2d.hpp>

class FakeRobotNode : public rclcpp::Node {
public:
  FakeRobotNode() : rclcpp::Node("fake_robot") {
    const std::chrono::milliseconds timer_period = std::chrono::milliseconds(500);
    rclcpp::QoS odom_qos{rclcpp::KeepLast{5}};
    odom_qos.best_effort();
    odom_qos.durability_volatile();
    odom_pub_ = create_publisher<nav_msgs::msg::Odometry>("odom", odom_qos);

    previous_time_ = now();

    timer_callback_group_ = create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

    cmd_callback_group_ = create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

    timer_ = create_wall_timer(
        timer_period,
        [this]() {
          double linear_x;
          double angular_z;

          {
            std::lock_guard<std::mutex> lock{cmd_mutex_};
            linear_x = cmd_linear_x_;
            angular_z = cmd_angular_z_;
          }

          const rclcpp::Time current_time = now();
          const double dt = (current_time - previous_time_).seconds();
          previous_time_ = current_time;
          const double dl = (linear_x - angular_z * track_ / 2.0) * dt;
          const double dr = (linear_x + angular_z * track_ / 2.0) * dt;
          pose_ = robomath::integrate_odom(pose_, dl, dr, track_);
          RCLCPP_INFO(get_logger(), "[timer tick] dt = %.4f x = %.2f y = %.2f theta = %.2f", dt,
                      pose_.x, pose_.y, pose_.theta);

          nav_msgs::msg::Odometry odom;
          odom.header.stamp = current_time;
          odom.header.frame_id = "odom";
          odom.child_frame_id = "base_link";

          odom.pose.pose.position.x = pose_.x;
          odom.pose.pose.position.y = pose_.y;
          odom.pose.pose.position.z = 0.0;

          const double half_theta = pose_.theta / 2.0;

          odom.pose.pose.orientation.x = 0.0;
          odom.pose.pose.orientation.y = 0.0;
          odom.pose.pose.orientation.z = std::sin(half_theta);
          odom.pose.pose.orientation.w = std::cos(half_theta);

          odom.twist.twist.linear.x = linear_x;
          odom.twist.twist.angular.z = angular_z;

          odom_pub_->publish(odom);
        },
        timer_callback_group_);

    rclcpp::QoS cmd_vel_qos{rclcpp::KeepLast{1}};
    cmd_vel_qos.reliable();
    cmd_vel_qos.durability_volatile();

    rclcpp::SubscriptionOptions cmd_options;
    cmd_options.callback_group = cmd_callback_group_;

    cmd_vel_sub_ = create_subscription<geometry_msgs::msg::Twist>(
        "cmd_vel", cmd_vel_qos,
        [this](geometry_msgs::msg::Twist::ConstSharedPtr msg) {
          const double linear_x = msg->linear.x;
          const double angular_z = msg->angular.z;

          {
            std::lock_guard<std::mutex> lock{cmd_mutex_};
            cmd_linear_x_ = linear_x;
            cmd_angular_z_ = angular_z;
          }

          RCLCPP_INFO(get_logger(), "[received cmd] linear = %.2f angular = %.2f", linear_x,
                      angular_z);
        },
        cmd_options);
  }

private:
  rclcpp::Time previous_time_;
  double cmd_linear_x_{0.0};
  double cmd_angular_z_{0.0};
  robomath::Pose2D pose_{0.0, 0.0, 0.0};
  double track_{0.4};
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_sub_;
  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
  std::mutex cmd_mutex_;
  rclcpp::CallbackGroup::SharedPtr timer_callback_group_;
  rclcpp::CallbackGroup::SharedPtr cmd_callback_group_;
};

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);

  std::shared_ptr<FakeRobotNode> node = std::make_shared<FakeRobotNode>();
  rclcpp::executors::MultiThreadedExecutor executor;
  executor.add_node(node);
  executor.spin();

  rclcpp::shutdown();
  return 0;
}