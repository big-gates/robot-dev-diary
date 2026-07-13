cmake_minimum_required(VERSION 3.16) # 이 버전 이상의 CMake로 열어라
project(my_turtlesim CXX) # 프로젝트 이름 + 언어(CXX=c++)

set(CMAKE_CXX_STANDARD 17) # g++의 -std=c++17에 해당
set(CMAKE_CXX_STANDARD_REQUIRED ON) # 17 못 쓰면 에러 (조용히 낮추지 않도록 하는것)
add_compile_options(-Wall -Wextra) # 이제 경고도 켠다 (모든 타깃에 적용)

# robomath 소스를 직접 컴파일에 포함 (아직 설치형 라이브러리가 아니므로)
add_executable(turtle
  main.cpp
  renderer.cpp
  sim_robot.cpp
  ../robomath/angles.cpp
  ../robomath/vec2.cpp
)

# "angles.hpp"를 ../robomath 에서 찾도록
target_include_directories(turtle PRIVATE ../robomath)
