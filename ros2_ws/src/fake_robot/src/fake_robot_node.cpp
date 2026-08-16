#include <algorithm>
#include <chrono>
#include <cmath>
#include <memory>
#include <mutex>

#include <fake_robot/action/move_to_point.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <rclcpp/callback_group.hpp>
#include <rclcpp/executors/multi_threaded_executor.hpp>
#include <rclcpp/logging.hpp>
#include <rclcpp/qos.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp/service.hpp>
#include <rclcpp/subscription_options.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include <rclcpp_action/types.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>
#include <robomath/angles.hpp>
#include <robomath/pose2d.hpp>
#include <robomath/vec2.hpp>
#include <std_srvs/srv/trigger.hpp>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <tf2_ros/buffer.hpp>
#include <tf2_ros/transform_broadcaster.hpp>
#include <tf2_ros/transform_listener.hpp>

class FakeRobotNode : public rclcpp_lifecycle::LifecycleNode {
public:
  using MoveToPoint = fake_robot::action::MoveToPoint;
  using GoalHandle = rclcpp_action::ServerGoalHandle<MoveToPoint>;

  FakeRobotNode() : rclcpp_lifecycle::LifecycleNode("fake_robot") {

    timer_callback_group_ = create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

    cmd_callback_group_ = create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
  }

  CallbackReturn on_configure(const rclcpp_lifecycle::State &previous_state) override {
    RCLCPP_INFO_STREAM(get_logger(),
                       "[lifecycle] on configure previous_state: " << previous_state.label());

    transform_broadcaster_ = std::make_shared<tf2_ros::TransformBroadcaster>(this);
    buffer_ = std::make_shared<tf2_ros::Buffer>(get_clock());
    transform_listener_ = std::make_shared<tf2_ros::TransformListener>(*buffer_, this);

    rclcpp::QoS odom_qos{rclcpp::KeepLast{5}};
    odom_qos.best_effort();
    odom_qos.durability_volatile();
    odom_pub_ = create_publisher<nav_msgs::msg::Odometry>("odom", odom_qos);

    reset_pose_service_ = create_service<std_srvs::srv::Trigger>(
        "reset_pose",
        [this](const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
               std::shared_ptr<std_srvs::srv::Trigger::Response> response) {
          (void)request;

          pose_ = robomath::Pose2D{0.0, 0.0, 0.0};

          response->success = true;
          response->message = "pose reset";

          RCLCPP_INFO(get_logger(), "[reset pose] x = 0.00 y = 0.00 theta = 0.00");
        },
        rclcpp::ServicesQoS{}, timer_callback_group_);

    move_to_point_server_ = rclcpp_action::create_server<MoveToPoint>(
        this, "move_to_point",
        [this](const rclcpp_action::GoalUUID &uuid, std::shared_ptr<const MoveToPoint::Goal> goal) {
          RCLCPP_INFO_STREAM(get_logger(), "[goal 요청] uuid: " << rclcpp_action::to_string(uuid)
                                                                << ", goal x: " << goal->target_x
                                                                << ", goal y: " << goal->target_y);
          if (goal_handle_ != nullptr && goal_handle_->is_active()) {
            RCLCPP_WARN_STREAM(get_logger(),
                               "[goal 거부] 이미 시행중인 goal이 있음 (진행 중: "
                                   << rclcpp_action::to_string(goal_handle_->get_goal_id()) << ")");
            return rclcpp_action::GoalResponse::REJECT;
          } else if (get_current_state().label() != "active") {
            return rclcpp_action::GoalResponse::REJECT;
          } else {
            return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
          }
        },
        [this](std::shared_ptr<GoalHandle> handle) {
          RCLCPP_INFO_STREAM(get_logger(), "[goal 취소] goal id: "
                                               << rclcpp_action::to_string(handle->get_goal_id()));
          return rclcpp_action::CancelResponse::ACCEPT;
        },
        [this](std::shared_ptr<GoalHandle> handle) {
          RCLCPP_INFO_STREAM(get_logger(), "[goal 실행] goal x: " << handle->get_goal()->target_x
                                                                  << ", goal y: "
                                                                  << handle->get_goal()->target_y);
          goal_handle_ = handle;
        },
        rcl_action_server_get_default_options(), timer_callback_group_);

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
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_activate(const rclcpp_lifecycle::State &previous_state) override {
    RCLCPP_INFO_STREAM(get_logger(),
                       "[lifecycle] on activate previous_state: " << previous_state.label());
    previous_time_ = now();

    timer_ = create_wall_timer(timer_period_, [this]() { on_timer(); }, timer_callback_group_);
    return rclcpp_lifecycle::LifecycleNode::on_activate(previous_state);
  }

  CallbackReturn on_deactivate(const rclcpp_lifecycle::State &previous_state) override {
    RCLCPP_INFO_STREAM(get_logger(),
                       "[lifecycle] on deactivate previous_state: " << previous_state.label());
    std::shared_ptr<MoveToPoint::Result> result = std::make_shared<MoveToPoint::Result>();
    if (goal_handle_ != nullptr && goal_handle_->is_active()) {
      result->set__final_x(pose_.x);
      result->set__final_y(pose_.y);
      result->set__message("deactivate 인하여 종료");
      result->set__success(false);
      goal_handle_->abort(result);
      goal_handle_.reset();
    }

    timer_.reset();

    {
      std::lock_guard<std::mutex> lock{cmd_mutex_};
      cmd_linear_x_ = 0.0;
      cmd_angular_z_ = 0.0;
    }
    return rclcpp_lifecycle::LifecycleNode::on_deactivate(previous_state);
  }

  CallbackReturn on_cleanup(const rclcpp_lifecycle::State &previous_state) override {
    RCLCPP_INFO_STREAM(get_logger(),
                       "[lifecycle] on cleanup previous_state: " << previous_state.label());

    odom_pub_.reset();
    reset_pose_service_.reset();
    move_to_point_server_.reset();
    cmd_vel_sub_.reset();
    transform_broadcaster_.reset();
    transform_listener_.reset();
    buffer_.reset();
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_shutdown(const rclcpp_lifecycle::State &previous_state) override {
    RCLCPP_INFO_STREAM(get_logger(),
                       "[lifecycle] on shutdown previous_state: " << previous_state.label());
    return CallbackReturn::SUCCESS;
  }

private:
  rclcpp::Time previous_time_;
  double cmd_linear_x_{0.0};
  double cmd_angular_z_{0.0};
  robomath::Pose2D pose_{0.0, 0.0, 0.0};
  double track_{0.4};
  double k_ang_{1.0};
  double k_lin_{0.5};
  double max_angular_{1.0};       // rad/s
  double max_linear_{0.5};        // m/s
  double goal_tolerance_{0.05};   // m
  double heading_tolerance_{0.3}; // rad (≈ 17°)

  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_sub_;
  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
  std::mutex cmd_mutex_;
  rclcpp::CallbackGroup::SharedPtr timer_callback_group_;
  rclcpp::CallbackGroup::SharedPtr cmd_callback_group_;
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr reset_pose_service_;
  rclcpp_action::Server<MoveToPoint>::SharedPtr move_to_point_server_;
  std::shared_ptr<GoalHandle> goal_handle_;
  const std::chrono::milliseconds timer_period_ = std::chrono::milliseconds(500);

  std::shared_ptr<tf2_ros::TransformBroadcaster> transform_broadcaster_;
  std::shared_ptr<tf2_ros::Buffer> buffer_;
  std::shared_ptr<tf2_ros::TransformListener> transform_listener_;

  void on_timer() {
    double linear_x;
    double angular_z;

    if (goal_handle_ != nullptr && goal_handle_->is_active()) {
      if (goal_handle_->is_canceling()) {
        // 취소 처리
        {
          std::lock_guard<std::mutex> lock{cmd_mutex_};
          cmd_linear_x_ = 0.0;
          cmd_angular_z_ = 0.0;
        }
        std::shared_ptr<MoveToPoint::Result> result = std::make_shared<MoveToPoint::Result>();
        result->set__success(false);
        result->set__final_x(pose_.x);
        result->set__final_y(pose_.y);
        result->set__message("취소 완료");
        goal_handle_->canceled(result);
        RCLCPP_INFO_STREAM(get_logger(),
                           "[goal 취소 완료] 최종 위치 (" << pose_.x << ", " << pose_.y << ")");
        goal_handle_.reset();
      } else {
        // 이동 처리(도달 판정 포함)
        std::shared_ptr<const MoveToPoint::Goal> goal = goal_handle_->get_goal();
        robomath::Vec2 target = {goal->target_x, goal->target_y};

        robomath::Vec2 current = {pose_.x, pose_.y};

        double remaining_distance = (target - current).norm();
        std::shared_ptr<MoveToPoint::Feedback> feedback = std::make_shared<MoveToPoint::Feedback>();
        feedback->set__remaining_distance(remaining_distance);
        goal_handle_->publish_feedback(feedback);

        if (remaining_distance < goal_tolerance_) {
          {
            std::lock_guard<std::mutex> lock{cmd_mutex_};
            cmd_linear_x_ = 0.0;
            cmd_angular_z_ = 0.0;
          }

          std::shared_ptr<MoveToPoint::Result> result = std::make_shared<MoveToPoint::Result>();
          result->set__success(true);
          result->set__final_x(pose_.x);
          result->set__final_y(pose_.y);
          result->set__message("목표 도달");
          goal_handle_->succeed(result);
          RCLCPP_INFO_STREAM(get_logger(),
                             "[goal 완료] 최종 위치 (" << pose_.x << ", " << pose_.y << ")");
          goal_handle_.reset();
        } else {
          double bearing =
              robomath::relative_bearing(pose_.x, pose_.y, pose_.theta, target.x, target.y);
          angular_z = std::clamp(k_ang_ * bearing, -max_angular_, max_angular_);
          if (std::abs(bearing) > heading_tolerance_) {
            linear_x = 0.0;
          } else {
            linear_x = std::clamp(k_lin_ * remaining_distance, 0.0, max_linear_);
          }

          {
            std::lock_guard<std::mutex> lock(cmd_mutex_);
            cmd_linear_x_ = linear_x;
            cmd_angular_z_ = angular_z;
          }
        }
      }
    }

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
    RCLCPP_INFO(get_logger(), "[timer tick] dt = %.4f x = %.2f y = %.2f theta = %.2f", dt, pose_.x,
                pose_.y, pose_.theta);

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

    geometry_msgs::msg::TransformStamped transform_stamped;
    transform_stamped.header.stamp = current_time;
    transform_stamped.header.frame_id = "odom";
    transform_stamped.child_frame_id = "base_link";
    transform_stamped.transform.translation.x = pose_.x;
    transform_stamped.transform.translation.y = pose_.y;
    transform_stamped.transform.translation.z = 0.0;
    transform_stamped.transform.rotation = odom.pose.pose.orientation;
    transform_broadcaster_->sendTransform(transform_stamped);

    geometry_msgs::msg::PointStamped laser_point_stamped;
    laser_point_stamped.header.frame_id = "laser";
    laser_point_stamped.point.x = 1.0;
    try {
      geometry_msgs::msg::PointStamped odom_point_stamped =
          buffer_->transform(laser_point_stamped, "odom");
      RCLCPP_INFO_THROTTLE(get_logger(), *get_clock(), 2000,
                           "[tf] laser(%.2f, %.2f, %.2f) -> odom(%.2f, %.2f, %.2f)",
                           laser_point_stamped.point.x, laser_point_stamped.point.y,
                           laser_point_stamped.point.z, odom_point_stamped.point.x,
                           odom_point_stamped.point.y, odom_point_stamped.point.z);
    } catch (const tf2::TransformException &ex) {
      RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 2000, "[tf] laser -> odom 변환 실패: %s",
                           ex.what());
    }
  }
};

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);

  std::shared_ptr<FakeRobotNode> node = std::make_shared<FakeRobotNode>();
  rclcpp::executors::MultiThreadedExecutor executor;
  executor.add_node(node->get_node_base_interface());
  executor.spin();

  rclcpp::shutdown();
  return 0;
}