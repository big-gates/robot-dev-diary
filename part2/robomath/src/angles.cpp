#include <robomath/angles.hpp>

#include <cmath>
#include <vector>

namespace robomath {

double deg2rad(double deg) { return deg * (kPi / 180.0); }

double rad2deg(double rad) { return rad / (kPi / 180.0); }

double wrap_to_pi(double rad) {
  double two_pi = 2.0 * kPi;
  double wrapped = std::fmod(rad + kPi, two_pi);
  if (wrapped < 0.0) { // fmod는 피제수 부호를 따라가므로 음수가 나올 수 있다
    wrapped += two_pi;
  }
  double result = wrapped - kPi;
  if (result <= -kPi) { // 경계: -π는 제외, +π는 포함 → (-π, π]
    result += two_pi;
  }
  return result;
}

double shortest_turn(double from_rad, double to_rad) {
  return wrap_to_pi(to_rad - from_rad);
}

double bearing_to(double rx, double ry, double tx, double ty) {
  return std::atan2(ty - ry, tx - rx); // atan2: 사분면 구분 + dx=0 안전
}

double relative_bearing(double rx, double ry, double rtheta, double tx,
                        double ty) {
  return wrap_to_pi(bearing_to(rx, ry, tx, ty) - rtheta);
}

double integrate_heading(double theta0, const std::vector<double> &omega,
                         double dt) {
  double theta = theta0;
  for (double w : omega) {
    theta += w * dt;
  }
  return wrap_to_pi(theta); // 누적된 결과를 정규화
}

bool in_fov(double rx, double ry, double rtheta, double tx, double ty,
            double fov_rad) {
  return std::fabs(relative_bearing(rx, ry, rtheta, tx, ty)) <= fov_rad / 2.0;
}

} // namespace robomath
