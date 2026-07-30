#include "svg_canvas.hpp"

#include <sstream>

namespace viz {

namespace {
// double을 소수점 2자리 문자열로 (SVG 좌표는 정밀도가 필요 없다)
std::string n(double v) {
  std::ostringstream os;
  os.precision(2);
  os << std::fixed << v;
  return os.str();
}
} // namespace

SvgCanvas::SvgCanvas(double world_w_m, double world_h_m, double px_per_m,
                     double margin_px)
    : px_per_m_(px_per_m), margin_px_(margin_px), world_h_m_(world_h_m) {
  width_px_ = world_w_m * px_per_m_ + 2.0 * margin_px_;
  height_px_ = world_h_m * px_per_m_ + 2.0 * margin_px_;
}

robomath::Vec2 SvgCanvas::to_px(robomath::Vec2 w) const {
  // x: 스케일 + 여백만
  // y: 뒤집기 — 월드는 위로 갈수록 y↑, SVG는 아래로 갈수록 y↑
  return robomath::Vec2{
      margin_px_ + w.x * px_per_m_,
      margin_px_ + (world_h_m_ - w.y) * px_per_m_,
  };
}

void SvgCanvas::line(robomath::Vec2 a, robomath::Vec2 b,
                     const std::string &color, double width_px,
                     const std::string &dash) {
  robomath::Vec2 pa = to_px(a);
  robomath::Vec2 pb = to_px(b);
  std::ostringstream os;
  os << "<line x1='" << n(pa.x) << "' y1='" << n(pa.y) << "' x2='" << n(pb.x)
     << "' y2='" << n(pb.y) << "' stroke='" << color << "' stroke-width='"
     << n(width_px) << "'";
  if (!dash.empty()) {
    os << " stroke-dasharray='" << dash << "'";
  }
  os << " stroke-linecap='round'/>\n";
  body_ += os.str();
}

void SvgCanvas::dot(robomath::Vec2 center, double r_px,
                    const std::string &fill) {
  robomath::Vec2 p = to_px(center);
  std::ostringstream os;
  os << "<circle cx='" << n(p.x) << "' cy='" << n(p.y) << "' r='" << n(r_px)
     << "' fill='" << fill << "'/>\n";
  body_ += os.str();
}

void SvgCanvas::cross(robomath::Vec2 center, double size_px,
                      const std::string &color, double width_px) {
  robomath::Vec2 p = to_px(center);
  double s = size_px;
  std::ostringstream os;
  os << "<line x1='" << n(p.x - s) << "' y1='" << n(p.y - s) << "' x2='"
     << n(p.x + s) << "' y2='" << n(p.y + s) << "' stroke='" << color
     << "' stroke-width='" << n(width_px) << "' stroke-linecap='round'/>\n";
  os << "<line x1='" << n(p.x - s) << "' y1='" << n(p.y + s) << "' x2='"
     << n(p.x + s) << "' y2='" << n(p.y - s) << "' stroke='" << color
     << "' stroke-width='" << n(width_px) << "' stroke-linecap='round'/>\n";
  body_ += os.str();
}

void SvgCanvas::polygon(const std::vector<robomath::Vec2> &pts,
                        const std::string &fill) {
  std::ostringstream os;
  os << "<polygon points='";
  for (const robomath::Vec2 &w : pts) {
    robomath::Vec2 p = to_px(w);
    os << n(p.x) << "," << n(p.y) << " ";
  }
  os << "' fill='" << fill << "'/>\n";
  body_ += os.str();
}

void SvgCanvas::text(robomath::Vec2 pos, const std::string &s,
                     const std::string &color, double size_px) {
  robomath::Vec2 p = to_px(pos);
  std::ostringstream os;
  os << "<text x='" << n(p.x) << "' y='" << n(p.y) << "' fill='" << color
     << "' font-family='monospace' font-size='" << n(size_px) << "'>" << s
     << "</text>\n";
  body_ += os.str();
}

std::string SvgCanvas::str() const {
  std::ostringstream os;
  os << "<svg xmlns='http://www.w3.org/2000/svg' width='" << n(width_px_)
     << "' height='" << n(height_px_) << "' viewBox='0 0 " << n(width_px_)
     << " " << n(height_px_) << "'>\n";
  os << "<rect width='100%' height='100%' fill='#0f1117'/>\n"; // 어두운 배경
  os << body_;
  os << "</svg>\n";
  return os.str();
}

} // namespace viz
