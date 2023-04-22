#include "matrices.h"
#include "renderer.h"

Eigen::Matrix4d Matrices::createScale(double scaleX, double scaleY, double scaleZ) {
    Eigen::Matrix4d scaleMatrix = Eigen::Matrix4d::Identity();
    scaleMatrix(0, 0) = scaleX;
    scaleMatrix(1, 1) = scaleY;
    scaleMatrix(2, 2) = scaleZ;
    return scaleMatrix;
}

Eigen::Matrix4d Matrices::createRotationX(double angle) {
    Eigen::Matrix4d rotationMatrix = Eigen::Matrix4d::Identity();
    double cosA = std::cos(angle);
    double sinA = std::sin(angle);
    rotationMatrix(1, 1) = cosA;
    rotationMatrix(1, 2) = -sinA;
    rotationMatrix(2, 1) = sinA;
    rotationMatrix(2, 2) = cosA;
    return rotationMatrix;
}

Eigen::Matrix4d Matrices::createRotationY(double angle) {
    Eigen::Matrix4d rotationMatrix = Eigen::Matrix4d::Identity();
    double cosA = std::cos(angle);
    double sinA = std::sin(angle);
    rotationMatrix(0, 0) = cosA;
    rotationMatrix(0, 2) = sinA;
    rotationMatrix(2, 0) = -sinA;
    rotationMatrix(2, 2) = cosA;
    return rotationMatrix;
}

Eigen::Matrix4d Matrices::createRotationZ(double angle) {
    Eigen::Matrix4d rotationMatrix = Eigen::Matrix4d::Identity();
    double cosA = std::cos(angle);
    double sinA = std::sin(angle);
    rotationMatrix(0, 0) = cosA;
    rotationMatrix(0, 1) = -sinA;
    rotationMatrix(1, 0) = sinA;
    rotationMatrix(1, 1) = cosA;
    return rotationMatrix;
}

Eigen::Matrix4d Matrices::createTranslation(double x, double y, double z) {
    Eigen::Matrix4d translationMatrix = Eigen::Matrix4d::Identity();
    translationMatrix(0, 3) = x;
    translationMatrix(1, 3) = y;
    translationMatrix(2, 3) = z;
    return translationMatrix;
}

Eigen::Matrix4d Matrices::createViewMatrix(const Eigen::Vector4d& eye, const Eigen::Vector4d& target, const Eigen::Vector4d& up) {
    Eigen::Vector4d forward = (eye - target).normalized();
    Eigen::Vector3d right3D = up.head<3>().cross(forward.head<3>()).normalized();
    Eigen::Vector4d right(right3D.x(), right3D.y(), right3D.z(), 1);
    Eigen::Vector3d newUp3D = forward.head<3>().cross(right.head<3>()).normalized();
    Eigen::Vector4d newUp(newUp3D.x(), newUp3D.y(), newUp3D.z(), 1);

    Eigen::Matrix4d orientation;
    orientation << right.x(), right.y(), right.z(), 0,
                   newUp.x(), newUp.y(), newUp.z(), 0,
                   forward.x(), forward.y(), forward.z(), 0,
                   0, 0, 0, 1;

    Eigen::Matrix4d translation;
    translation << 1, 0, 0, -eye.x(),
                   0, 1, 0, -eye.y(),
                   0, 0, 1, -eye.z(),
                   0, 0, 0, 1;

    return orientation * translation;
}

Eigen::Matrix4d Matrices::createPerspectiveProjection(double fov, double aspectRatio, double near, double far) {
    double tanHalfFov = std::tan(fov / 2);
    Eigen::Matrix4d perspectiveMatrix = Eigen::Matrix4d::Zero();
    perspectiveMatrix(0, 0) = 1 / (aspectRatio * tanHalfFov);
    perspectiveMatrix(1, 1) = 1 / tanHalfFov;
    perspectiveMatrix(2, 2) = -(far + near) / (far - near);
    perspectiveMatrix(3, 2) = -2 * far * near / (far - near);
    perspectiveMatrix(2, 3) = -1;
    perspectiveMatrix(3, 3) = 0;
    return perspectiveMatrix;
}
