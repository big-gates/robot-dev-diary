cmake_minimum_required(VERSION 3.16) # 이 버전 이상의 CMake로 열어라
project(robomath CXX) # 프로젝트 이름 + 언어(CXX=c++)

set(CMAKE_CXX_STANDARD 17) # g++의 -std=c++17에 해당
set(CMAKE_CXX_STANDARD_REQUIRED ON) # 17 못 쓰면 에러 (조용히 낮추지 않도록 하는것)
add_compile_options(-Wall -Wextra) # 이제 경고도 켠다 (모든 타깃에 적용)

# google test 가져오기
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
)
FetchContent_MakeAvailable(googletest)

enable_testing()

# 라이브러리 타깃
add_library(robomath angles.cpp vec2.cpp)

# 실행 타깃: 테스트 프로그램
add_executable(test_angles test_angles.cpp)
# 연결: test_angles 는 robomath를 가져다 쓴다
target_link_libraries(test_angles PRIVATE robomath GTest::gtest_main)
add_test(NAME test_angles COMMAND test_angles)

add_executable(test_vec2 test_vec2.cpp)
target_link_libraries(test_vec2 PRIVATE robomath GTest::gtest_main)
add_test(NAME test_vec2 COMMAND test_vec2)

add_executable(drift experiments/drift.cpp)
target_link_libraries(drift PRIVATE robomath)
target_include_directories(drift PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})