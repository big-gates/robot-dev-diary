#include "svg_canvas.hpp"

#include <robomath/angles.hpp>
#include <robomath/pose2d.hpp>
#include <robomath/vec2.hpp>

#include <cmath>
#include <fstream>
#include <iostream>
#include <random> // 노이즈용 (3단계에서)
#include <string>
#include <vector>

using namespace robomath;

int main() {
  const double track = 0.4;
  const double dl = 0.04;    // 왼바퀴 증분
  const double dr = 0.06;    // 오른바퀴 증분(dr > dl -> 원)
  const int num_steps = 150; // 스텝 수 (한 바퀴 조금 넘게)
  const double start_x = 1.2;
  const double start_y = 0.3;
  const double start_theta = 0.0;
  Pose2D true_pose{start_x, start_y, start_theta};

  std::mt19937 rng(42);                               // 난수 생성기, 시드 42(고정 = 재현가능)
  std::normal_distribution<double> noise(0.0, 0.005); // 정규분포: 평균0, 표준편차 0.005

  Pose2D est_pose{start_x, start_y, start_theta};
  std::vector<Vec2> est_path{{est_pose.x, est_pose.y}};

  std::vector<Vec2> true_path{{true_pose.x, true_pose.y}};

  for (int i = 0; i < num_steps; ++i) {
    true_pose = integrate_odom(true_pose, dl, dr, track);
    true_path.push_back(Vec2{true_pose.x, true_pose.y});

    double nl = dl + noise(rng); // 노이즈 낀 왼바퀴 읽음
    double nr = dr + noise(rng); // 노이즈 낀 오른바퀴 읽음

    est_pose = integrate_odom(est_pose, nl, nr, track);
    est_path.push_back(Vec2{est_pose.x, est_pose.y});
  }

  const double drift = std::hypot(true_pose.x - est_pose.x, true_pose.y - est_pose.y);
  std::cout << "최종 드리프트: " << drift << " m\n";

  std::cout << "true 끝: (" << true_pose.x << ", " << true_pose.y << ", " << true_pose.theta
            << ")\n";

  viz::SvgCanvas cv(6.0, 4.0, 110.0); // 6m x 4m 공간

  const std::string true_path_color = "#0000FF";
  const std::string est_path_color = "#FF0000";
  for (std::size_t i = 1; i < true_path.size(); ++i) {
    cv.line(true_path[i - 1], true_path[i], true_path_color);
    cv.line(est_path[i - 1], est_path[i], est_path_color);
  }

  const Pose2D table{4.5, 2.0, kPi};
  const Pose2D ap = approach_pose(table, 0.3);
  const Vec2 table_pos{table.x, table.y};
  const Vec2 approach_pos{ap.x, ap.y};
  const double heading_length = 0.1;
  const std::string table_color = "#2ECC71";
  const std::string approach_color = "#F1C40F";

  std::cout << "table:\t(" << table.x << ", " << table.y << ", " << table.theta << ")\n";
  std::cout << "approach:\t(" << ap.x << ", " << ap.y << ", " << ap.theta << ")\n";
  std::cout << "거리:\t" << std::hypot(table.x - ap.x, table.y - ap.y) << "\n";

  const double move_heading = bearing_to(true_pose.x, true_pose.y, ap.x, ap.y);
  const double first_turn = shortest_turn(true_pose.theta, move_heading);
  const double move_distance = std::hypot(true_pose.x - ap.x, true_pose.y - ap.y);
  const double final_turn = shortest_turn(move_heading, ap.theta);

  std::cout << "이동 헤딩:\t" << move_heading << " rad\t (" << rad2deg(move_heading) << "°)\n";
  std::cout << "첫 회전량:\t" << first_turn << " rad\t (" << rad2deg(first_turn) << "°)\n";
  std::cout << "이동 거리:\t" << move_distance << " m\n";
  std::cout << "최종 회전:\t" << final_turn << " rad\t (" << rad2deg(final_turn) << "°)\n";

  const Pose2D s0{true_pose.x, true_pose.y, true_pose.theta};
  const Pose2D s1{s0.x, s0.y, move_heading};
  const Vec2 s2_position = s1.transform({move_distance, 0.0});
  const Pose2D s2{s2_position.x, s2_position.y, move_heading};
  const Pose2D s3{s2.x, s2.y, ap.theta};

  std::cout << "S1: (" << s1.x << ", " << s1.y << ", " << s1.theta << ")\n";
  std::cout << "S2: (" << s2.x << ", " << s2.y << ", " << s2.theta << ")\n";
  std::cout << "S3: (" << s3.x << ", " << s3.y << ", " << s3.theta << ")\n";
  std::cout << "도착 위치 오차: " << std::hypot(s3.x - ap.x, s3.y - ap.y) << " m\n";

  const std::string line_color = "#00D4FF";
  const double state_heading_length = 0.32;
  const double width_px = 2.0;
  const std::string dash = "8 4";

  const Vec2 s0_position{s0.x, s0.y};
  const Vec2 s1_position{s1.x, s1.y};

  cv.line(s1_position, s2_position, line_color, width_px, dash);
  cv.line(s0_position, s0.transform({state_heading_length, 0.0}), "#CBD5E1", width_px, "4 3");
  cv.line(s1_position, s1.transform({state_heading_length, 0.0}), line_color, width_px);
  cv.line(s2_position, s2.transform({state_heading_length, 0.0}), "#CBD5E1", width_px, "4 3");

  cv.line(table_pos, table.transform({heading_length, 0.0}), table_color, 2.0);
  cv.line(approach_pos, ap.transform({heading_length, 0.0}), approach_color, 2.0);
  cv.text({4.55, 2.15}, "table", table_color);
  cv.text({3.65, 1.85}, "approach", approach_color);

  cv.dot(true_path.back(), 6.0, true_path_color);
  cv.dot(est_path.back(), 6.0, est_path_color);

  cv.dot(table_pos, 6.0, table_color);
  cv.dot(approach_pos, 6.0, approach_color);

  cv.line({0.3, 3.1}, {0.5, 3.1}, true_path_color);
  cv.text({0.6, 3.1}, "true odometry", "#FFFFFF");

  cv.line({0.3, 3.35}, {0.5, 3.35}, est_path_color);
  cv.text({0.6, 3.35}, "noisy estimate", "#FFFFFF");

  cv.line({0.3, 3.6}, {0.5, 3.6}, line_color, width_px, dash);
  cv.text({0.6, 3.6}, "planned motion", "#FFFFFF");

  cv.line({0.3, 3.85}, {0.5, 3.85}, "#CBD5E1", width_px, "4 3");
  cv.text({0.6, 3.85}, "heading before turn", "#FFFFFF");

  std::ofstream("odom.svg") << cv.str();
  std::cout << "odom.svg 저장 완료\n";

  return 0;
}
