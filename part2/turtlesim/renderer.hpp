#pragma once

#include <string>

// 터미널 색·제어 코드 (main.cpp도 kClear를 쓰므로 헤더에)
namespace ansi {
constexpr const char *kReset = "\033[0m";
constexpr const char *kCyan = "\033[36m";  // 로봇
constexpr const char *kGreen = "\033[32m"; // 목표
constexpr const char *kDim = "\033[90m";   // 테두리
constexpr const char *kClear = "\033[2J\033[H"; // 화면 지우고 커서 맨 위로
} // namespace ansi

struct Pose2D {
  double x = 0.0;
  double y = 0.0;
  double theta = 0.0;
};

struct ViewConfig {
  int width = 40;          // 가로 칸 수
  int height = 20;         // 세로 칸 수
  double cell_size = 0.25; // 한 칸 = 몇 미터
};

// 로봇과 목표를 격자에 그려서 화면에 출력할 문자열로 반환
std::string render(const Pose2D &robot, double goal_x, double goal_y,
                   const ViewConfig &view);
