cmake_minimum_required(VERSION 3.16)
project(fake_robot CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)

add_subdirectory(
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../part2/robomath
  ${CMAKE_CURRENT_BINARY_DIR}/robomath_build
)

add_executable(fake_robot_node src/fake_robot_node.cpp)
ament_target_dependencies(fake_robot_node rclcpp geometry_msgs nav_msgs)
target_link_libraries(fake_robot_node robomath)

install(TARGETS fake_robot_node DESTINATION lib/${PROJECT_NAME})

ament_package()
