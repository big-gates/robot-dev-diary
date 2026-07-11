#pragma once // 이 파일 두번 include 돼도 한번만 처리해라 헤더의 필수 첫 줄

#include <vector>
constexpr double kPi = 3.14159265358979323846;

double deg2rad(double deg); // 몸통 없이 선언만
double rad2deg(double rad);
double wrap_to_pi(double rad);
double shortest_turn(double from_rad, double to_rad);

// 지도 기준: 로봇 위치(rx,ry)에서 목표(tx,ty)를 봤을 때 "지도의 동쪽(0도) 기준"
// 방위각
double bearing_to(double rx, double ry, double tx, double ty);

// 로봇 기준: 로봇이 지금 rtheta 방향을 보고 있을 때, 목표가 "내 정면 기준" 몇
// 도 왼쪽/오른쪽인가
double relative_bearing(double rx, double ry, double rtheta, double tx,
                        double ty);

double integrate_heading(double theta0, const std::vector<double> &omega, double dt);

bool in_fov(double rx, double ry, double rtheta, double tx, double ty, double fov_rad);