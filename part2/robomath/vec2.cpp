#include "vec2.hpp"
#include <cmath>

Vec2 project_forward(double x, double y, double theta, double dist) {
    return Vec2{
        x + dist * std::cos(theta),
        y + dist * std::sin(theta) 
    };
}