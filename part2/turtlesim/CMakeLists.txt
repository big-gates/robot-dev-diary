cmake_minimum_required(VERSION 3.16)
project(my_turtlesim CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

# 이웃 프로젝트인 robomath를 하위 빌드로 포함
# → robomath 타깃과 그 PUBLIC include 경로를 그대로 물려받는다
add_subdirectory(../robomath robomath_build)

add_executable(turtle
  main.cpp
  renderer.cpp
  sim_robot.cpp
)

# robomath만 링크하면 #include <robomath/angles.hpp> 가 자동으로 열린다
target_link_libraries(turtle PRIVATE robomath)
