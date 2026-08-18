cmake_minimum_required(VERSION 3.16)
project(fake_robot C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)

add_compile_options(-Wall -Wextra -Wpedantic)

add_subdirectory(
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../part2/robomath
  ${CMAKE_CURRENT_BINARY_DIR}/robomath_build
)

rosidl_generate_interfaces(${PROJECT_NAME} "action/MoveToPoint.action")
ament_export_dependencies(rosidl_default_runtime)

add_executable(fake_robot_node src/fake_robot_node.cpp)
ament_target_dependencies(fake_robot_node rclcpp geometry_msgs nav_msgs std_srvs rclcpp_action rclcpp_lifecycle tf2_ros tf2_geometry_msgs)
rosidl_get_typesupport_target(cpp_typesupport_target ${PROJECT_NAME} "rosidl_typesupport_cpp")
target_link_libraries(fake_robot_node robomath "${cpp_typesupport_target}")

add_executable(teleop_node src/teleop_node.cpp)
ament_target_dependencies(teleop_node rclcpp geometry_msgs)

install(TARGETS fake_robot_node DESTINATION lib/${PROJECT_NAME})
install(TARGETS teleop_node DESTINATION lib/${PROJECT_NAME})
install(DIRECTORY urdf worlds DESTINATION share/${PROJECT_NAME})

ament_package()
