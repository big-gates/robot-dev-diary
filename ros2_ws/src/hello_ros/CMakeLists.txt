cmake_minimum_required(VERSION 3.16)
project(hello_ros)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

add_executable(hello_node src/hello_node.cpp)
ament_target_dependencies(hello_node rclcpp std_msgs)

install(TARGETS hello_node DESTINATION lib/${PROJECT_NAME})

ament_package()
