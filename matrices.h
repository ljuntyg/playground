#ifndef MATRICES_H
#define MATRICES_H

#include <Eigen>

class Matrices {
public:
    static Eigen::Matrix4d createScale(double scaleX, double scaleY, double scaleZ);
    static Eigen::Matrix4d createRotationX(double angle);
    static Eigen::Matrix4d createRotationY(double angle);
    static Eigen::Matrix4d createRotationZ(double angle);
    static Eigen::Matrix4d createRotationCustom(const Eigen::Vector3d& axis, double angle);
    static Eigen::Matrix4d createTranslation(double x, double y, double z);
    static Eigen::Matrix4d createViewMatrix(const Eigen::Vector4d &cameraPos, const Eigen::Vector4d &targetPos, const Eigen::Vector4d &upVector);
    static Eigen::Matrix4d createPerspectiveProjection(double fov, double aspectRatio, double near, double far);
};

#endif // MATRICES_H
