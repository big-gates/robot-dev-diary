#pragma once

#include <robomath/vec2.hpp>
#include <string>
#include <vector>

namespace viz {

// 월드 좌표(m, y가 위로 증가)를 SVG 픽셀(y가 아래로 증가)로 바꿔 그려주는 캔버스.
// robomath::Vec2를 그대로 받으므로, 우리가 만든 수학 결과를 곧바로 그림으로 옮긴다.
//
// 사용법:
//   SvgCanvas cv(6.0, 4.0);          // 6m x 4m 공간
//   cv.line({0,0}, {1,1}, "#fff");   // 월드 좌표로 그리기
//   std::ofstream("out.svg") << cv.str();
class SvgCanvas {
public:
  // world_w/h_m: 그릴 실제 공간 크기(m)
  // px_per_m   : 1m를 몇 픽셀로 (클수록 큰 그림)
  // margin_px  : 테두리 여백(px)
  SvgCanvas(double world_w_m, double world_h_m, double px_per_m = 100.0,
            double margin_px = 24.0);

  // 선분 a→b. dash="" 면 실선, "8 4" 처럼 주면 점선.
  void line(robomath::Vec2 a, robomath::Vec2 b, const std::string &color,
            double width_px = 1.0, const std::string &dash = "");

  // 채워진 원. 반지름은 픽셀 단위 — 점 크기를 화면 기준으로 고정하기 위함.
  void dot(robomath::Vec2 center, double r_px, const std::string &fill);

  // X자 마커 (교점/코너 강조용)
  void cross(robomath::Vec2 center, double size_px, const std::string &color,
             double width_px = 2.0);

  // 채워진 다각형 (로봇 삼각형 등). 점들은 월드 좌표.
  void polygon(const std::vector<robomath::Vec2> &pts, const std::string &fill);

  // 텍스트 (범례 등). pos는 월드 좌표.
  void text(robomath::Vec2 pos, const std::string &s, const std::string &color,
            double size_px = 14.0);

  std::string str() const; // 완성된 <svg>…</svg> 문자열

private:
  robomath::Vec2 to_px(robomath::Vec2 w) const; // 월드 → 픽셀

  double px_per_m_;
  double margin_px_;
  double world_h_m_;
  double width_px_;
  double height_px_;
  std::string body_; // 누적되는 SVG 요소들
};

} // namespace viz
