#ifndef STRUCTS_H
#define STRUCTS_H

#include <Eigen>

struct Triangle {
    Eigen::Vector4d v1;
    Eigen::Vector4d v2;
    Eigen::Vector4d v3;

    double shadeVal;

    Triangle() : v1(Eigen::Vector4d::Zero()), v2(Eigen::Vector4d::Zero()), v3(Eigen::Vector4d::Zero()), shadeVal(0.5) {}
    Triangle(Eigen::Vector4d v1, Eigen::Vector4d v2, Eigen::Vector4d v3, double shadeVal = 0.5) : v1(v1), v2(v2), v3(v3), shadeVal(shadeVal) {}
};

/* struct ClipPlane {
    Eigen::Vector4d normal;
    double distance;

    ClipPlane() : normal(Eigen::Vector4d::Zero()), distance(0.0) {}
    ClipPlane(Eigen::Vector4d normal, double distance) : normal(normal), distance(distance) {}
}; */

#endif // STRUCTS_H