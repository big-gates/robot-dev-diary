#pragma once

namespace robomath {

// 2D 벡터 — 좌표, 변위, 속도, LiDAR 점 등 모든 것의 부품
struct Vec2 {
  double x = 0.0;
  double y = 0.0;

  Vec2 operator+(const Vec2 &o) const;
  Vec2 operator-(const Vec2 &o) const; // b - a = "a에서 b로 가는 화살표"
  Vec2 operator*(double s) const;      // 스칼라 곱 (방향 유지, 길이만 조절)

  // 내적: >0 같은 방향 / =0 수직 / <0 반대 방향
  double dot(const Vec2 &o) const;

  double norm() const; // 길이 (피타고라스)

  // 길이를 1로 만든 순수 방향 벡터
  // 정책: 영벡터는 방향이 없으므로 (0,0) 반환 (NaN 전파 방지)
  Vec2 normalized() const;
};

// 현재 위치에서 theta 방향으로 dist 만큼 전진한 좌표
Vec2 project_forward(double x, double y, double theta, double dist);

} // namespace robomath
