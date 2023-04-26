#ifndef STRUCTS_H
#define STRUCTS_H

#include <Eigen>

struct Triangle {
    Eigen::Vector4d v1;
    Eigen::Vector4d v2;
    Eigen::Vector4d v3;

    SDL_Color color;

    Triangle() : v1(Eigen::Vector4d::Zero()), v2(Eigen::Vector4d::Zero()), v3(Eigen::Vector4d::Zero()), color({100, 100, 100, 255}) {}
    Triangle(Eigen::Vector4d v1, Eigen::Vector4d v2, Eigen::Vector4d v3, SDL_Color color = {100, 100, 100, 255}) : v1(v1), v2(v2), v3(v3), color(color) {}
};

/* struct ClipPlane {
    Eigen::Vector4d normal;
    double distance;

    ClipPlane() : normal(Eigen::Vector4d::Zero()), distance(0.0) {}
    ClipPlane(Eigen::Vector4d normal, double distance) : normal(normal), distance(distance) {}
}; */

#endif // STRUCTS_H