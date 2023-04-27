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

struct Mesh {
    std::vector<Eigen::Vector4d> vertices;
    std::vector<size_t> indices;

    Mesh() = default;
    Mesh(const std::vector<Eigen::Vector4d>& vertices, const std::vector<size_t>& indices) : vertices(vertices), indices(indices) {}
};

struct ClipPlane {
    Eigen::Vector4d planePoint;
    Eigen::Vector4d planeNormal;

    ClipPlane() : planePoint(Eigen::Vector4d::Zero()), planeNormal(Eigen::Vector4d::Zero()) {}
    ClipPlane(Eigen::Vector4d planePoint, Eigen::Vector4d planeNormal) : planePoint(planePoint), planeNormal(planeNormal) {}
};

#endif // STRUCTS_H