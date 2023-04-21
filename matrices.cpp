#include "Matrices.h"

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

Eigen::Matrix4d Matrices::createViewMatrix(const Eigen::Vector4d &cameraPos, const Eigen::Vector4d &targetPos, const Eigen::Vector4d &upVector) {
    Eigen::Vector3d zAxis = (cameraPos.head<3>() - targetPos.head<3>()).normalized();
    Eigen::Vector3d xAxis = upVector.head<3>().cross(zAxis).normalized();
    Eigen::Vector3d yAxis = zAxis.cross(xAxis);

    Eigen::Matrix4d viewMatrix = Eigen::Matrix4d::Identity();
    viewMatrix(0, 0) = xAxis.x();
    viewMatrix(0, 1) = yAxis.x();
    viewMatrix(0, 2) = zAxis.x();
    viewMatrix(1, 0) = xAxis.y();
    viewMatrix(1, 1) = yAxis.y();
    viewMatrix(1, 2) = zAxis.y();
    viewMatrix(2, 0) = xAxis.z();
    viewMatrix(2, 1) = yAxis.z();
    viewMatrix(2, 2) = zAxis.z();
    viewMatrix(0, 3) = -xAxis.dot(cameraPos.head<3>());
    viewMatrix(1, 3) = -yAxis.dot(cameraPos.head<3>());
    viewMatrix(2, 3) = -zAxis.dot(cameraPos.head<3>());
    return viewMatrix;
}

Eigen::Matrix4d Matrices::createPerspectiveProjection(double fov, double aspectRatio, double near, double far) {
    double tanHalfFov = std::tan(fov / 2.0);
    Eigen::Matrix4d perspectiveMatrix = Eigen::Matrix4d::Zero();
    perspectiveMatrix(0, 0) = 1.0 / (aspectRatio * tanHalfFov);
    perspectiveMatrix(1, 1) = 1.0 / tanHalfFov;
    perspectiveMatrix(2, 2) = -(far + near) / (far - near);
    perspectiveMatrix(3, 2) = -2.0 * far * near / (far - near);
    perspectiveMatrix(2, 3) = -1.0;
    return perspectiveMatrix;
}
