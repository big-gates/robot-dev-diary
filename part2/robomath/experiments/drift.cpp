#include <iostream>
#include <vector>
#include <robomath/angles.hpp>

using namespace robomath;

int main() {
    const double bias = 0.001; // 자이로 바이어스: 초당 0.001 rad 거짓말
    const double dt = 0.01; // 100Hz 샘플링  

    std::cout << "정지한 로봇의 자이로 드리프트 (bias = " << bias << " rad/s)\n";                                                                                                                                
    std::cout << "시간(초)\t오차(도)\n";

    for(int seconds : {10, 60, 300, 600, 1800, 3600}){
        int n = static_cast<int>(seconds / dt);
        std::vector<double> omega(n, bias);
        double theta = integrate_heading(0.0, omega, dt);
        std::cout << seconds << "\t\t" << rad2deg(theta) << "\n";
    }
}