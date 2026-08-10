cmake_minimum_required(VERSION 3.16)
project(robomath CXX)

# 디버깅 기본값: 빌드 타입 미지정 시 Debug(-g) — VS Code 중단점이 바로 잡히게
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

option(ROBOMATH_BUILD_TESTS "Build robomath tests and experiments" ${PROJECT_IS_TOP_LEVEL})

if(ROBOMATH_BUILD_TESTS)
  # ── 의존성: GoogleTest ──
  include(FetchContent)
  FetchContent_Declare(
      googletest
      GIT_REPOSITORY https://github.com/google/googletest.git
      GIT_TAG v1.15.2
  )
  FetchContent_MakeAvailable(googletest)

  enable_testing()
endif()

# ── 라이브러리 타깃 ──
add_library(robomath
  src/angles.cpp
  src/vec2.cpp
  src/mat2.cpp
  src/mat3.cpp
  src/line.cpp
  src/pose2d.cpp
  src/grid.cpp
)

# PUBLIC = 나를 링크하는 타깃(테스트, turtlesim)도 이 헤더 경로를 물려받는다
# → 다른 프로젝트에서 #include <robomath/angles.hpp> 로 쓸 수 있게 된다
target_include_directories(robomath PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(ROBOMATH_BUILD_TESTS)
  # ── 테스트 타깃들 (3종 세트: 빌드 → 링크 → ctest 등록) ──
  add_executable(test_angles test/test_angles.cpp)
  target_link_libraries(test_angles PRIVATE robomath GTest::gtest_main)
  add_test(NAME test_angles COMMAND test_angles)

  add_executable(test_vec2 test/test_vec2.cpp)
  target_link_libraries(test_vec2 PRIVATE robomath GTest::gtest_main)
  add_test(NAME test_vec2 COMMAND test_vec2)

  # 2-2 구현하면 주석 해제
  add_executable(test_mat2 test/test_mat2.cpp)
  target_link_libraries(test_mat2 PRIVATE robomath GTest::gtest_main)
  add_test(NAME test_mat2 COMMAND test_mat2)

  add_executable(test_mat3 test/test_mat3.cpp)
  target_link_libraries(test_mat3 PRIVATE robomath GTest::gtest_main)
  add_test(NAME test_mat3 COMMAND test_mat3)

  add_executable(test_line test/test_line.cpp)
  target_link_libraries(test_line PRIVATE robomath GTest::gtest_main)
  add_test(NAME test_line COMMAND test_line)

  add_executable(test_pose2d test/test_pose2d.cpp)
  target_link_libraries(test_pose2d PRIVATE robomath GTest::gtest_main)
  add_test(NAME test_pose2d COMMAND test_pose2d)

  add_executable(test_grid test/test_grid.cpp)
  target_link_libraries(test_grid PRIVATE robomath GTest::gtest_main)
  add_test(NAME test_grid COMMAND test_grid)

  # ── 실험 (채점이 아니라 관찰이 목적 — add_test 없음) ──
  add_executable(drift experiments/drift.cpp)
  target_link_libraries(drift PRIVATE robomath)
endif()
