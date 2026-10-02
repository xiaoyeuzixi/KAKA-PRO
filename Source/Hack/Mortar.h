#pragma once
#include <cmath>
#include <numbers>

#define _USE_MATH_DEFINES

namespace Mortar {

    double calculateAngle(double distance, double height) {
        // 计算仰角（弧度）
        double radians = std::atan2(height, distance);

        // 转换为角度
        double degrees = radians * (180.0 / std::numbers::pi);
        return degrees;
    }


    double calculateR(double L, double betaDeg, double v0 = 83.0, double g = 9.8) {
        // 将角度转换为弧度
        double betaRad = betaDeg * (std::numbers::pi / 180.0);
        double tanBeta = std::tan(betaRad);

        // 计算中间量
        double v0SquaredOverG = (v0 * v0) / g;
        double v0Fourth = std::pow(v0, 4);
        double gSquared = g * g;

        // 计算根号内的表达式
        double sqrtExpression = (v0Fourth / gSquared)
            - ((2 * L * v0 * v0 * tanBeta) / g)
            - (L * L);

        if (sqrtExpression < 0) {
            return -1.0; // 使用负数表示无效结果
        }

        double sqrtResult = std::sqrt(sqrtExpression);
        double numerator = L + tanBeta * (v0SquaredOverG - sqrtResult);
        double denominator = 1.0 + tanBeta * tanBeta;

        return numerator / denominator;
    }

    double getDistance(double d, double h) {
        double r = calculateAngle(d, h);
        double m = calculateR(d, r);
        return m >= 0 ? m : 0.0; // 处理无效结果
    }
}