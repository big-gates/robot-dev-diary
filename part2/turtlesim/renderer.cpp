#include "renderer.hpp"

#include "angles.hpp"

#include <cmath>
#include <cstdio>
#include <vector>

namespace {

// 헤딩을 8방향 화살표로. 인덱스 0=동, 반시계로 45도씩
const char *kArrows[8] = {"▶", "◥", "▲", "◤", "◀", "◣", "▼", "◢"};

using Grid = std::vector<std::vector<std::string>>;

int arrow_index(double theta) {
  double t = wrap_to_pi(theta);
  int idx = static_cast<int>(std::round(t / (kPi / 4.0))); // -4 ~ +4
  if (idx < 0) {
    idx += 8; // -1 → 7 (남동)
  }
  return idx % 8; // +4(=π, 서쪽)는 그대로 4
}

// world 좌표(m) → 격자 칸에 글리프 찍기. 화면 밖이면 무시
void plot(Grid &grid, const ViewConfig &view, double x, double y,
          const std::string &glyph) {
  int col = static_cast<int>(x / view.cell_size);
  // y축 뒤집기: 수학은 y가 클수록 위, 터미널은 row가 클수록 아래
  int row = view.height - 1 - static_cast<int>(y / view.cell_size);
  if (col >= 0 && col < view.width && row >= 0 && row < view.height) {
    grid[row][col] = glyph;
  }
}

std::string colored(const char *color, const std::string &glyph) {
  return std::string(color) + glyph + ansi::kReset;
}

std::string repeat(const std::string &s, int n) {
  std::string out;
  for (int i = 0; i < n; ++i) {
    out += s;
  }
  return out;
}

} // namespace

std::string render(const Pose2D &robot, double goal_x, double goal_y,
                   const ViewConfig &view) {
  // 1. 빈 격자 (배경은 공백)
  Grid grid(view.height, std::vector<std::string>(view.width, " "));

  // 2. 목표 → 3. 로봇 순서로 찍기 (겹치면 로봇이 위에 보이도록)
  plot(grid, view, goal_x, goal_y, colored(ansi::kGreen, "◎"));
  plot(grid, view, robot.x, robot.y,
       colored(ansi::kCyan, kArrows[arrow_index(robot.theta)]));

  // 4. 테두리 + 격자 + 상태줄 조립
  std::string out;
  const std::string bar = repeat("─", view.width);

  out += ansi::kDim;
  out += "┌" + bar + "┐\n";
  out += ansi::kReset;

  for (const auto &row : grid) {
    out += ansi::kDim;
    out += "│";
    out += ansi::kReset;
    for (const auto &cell : row) {
      out += cell;
    }
    out += ansi::kDim;
    out += "│\n";
    out += ansi::kReset;
  }

  out += ansi::kDim;
  out += "└" + bar + "┘\n";
  out += ansi::kReset;

  double dist = std::hypot(goal_x - robot.x, goal_y - robot.y);
  double bearing =
      relative_bearing(robot.x, robot.y, robot.theta, goal_x, goal_y);

  char line[160];
  std::snprintf(line, sizeof(line),
                "  pos (%.2f, %.2f)  heading %+6.1f°  |  goal dist %.2fm  "
                "bearing %+6.1f°\n",
                robot.x, robot.y, rad2deg(robot.theta), dist,
                rad2deg(bearing));
  out += line;

  return out;
}
