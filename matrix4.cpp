#include "Matrix4.h"
#include <cmath>

Matrix4::Matrix4() {
    m_data = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
}

Matrix4::Matrix4(const std::array<float, 16>& data) {
    m_data = data;
}

Matrix4 Matrix4::operator*(const Matrix4& other) const {
    Matrix4 result;
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            result(row, col) = 0;
            for (int k = 0; k < 4; ++k) {
                result(row, col) += (*this)(row, k) * other(k, col);
            }
        }
    }
    return result;
}

float Matrix4::operator()(int row, int col) const {
    return m_data[row * 4 + col];
}

float& Matrix4::operator()(int row, int col) {
    return m_data[row * 4 + col];
}

Matrix4 Matrix4::createTranslation(float x, float y, float z) {
    Matrix4 translation = Matrix4();
    translation(0, 3) = x;
    translation(1, 3) = y;
    translation(2, 3) = z;
    return translation;
}

Matrix4 Matrix4::createScale(float x, float y, float z) {
    Matrix4 scale = Matrix4();
    scale(0, 0) = x;
    scale(1, 1) = y;
    scale(2, 2) = z;
    return scale;
}

Matrix4 Matrix4::createRotationX(float angle) {
    Matrix4 rotation = Matrix4();
    float c = std::cos(angle);
    float s = std::sin(angle);
    rotation(1, 1) = c;
    rotation(1, 2) = -s;
    rotation(2, 1) = s;
    rotation(2, 2) = c;
    return rotation;
}

Matrix4 Matrix4::createRotationY(float angle) {
    Matrix4 rotation = Matrix4();
    float c = std::cos(angle);
    float s = std::sin(angle);
    rotation(0, 0) = c;
    rotation(0, 2) = s;
    rotation(2, 0) = -s;
    rotation(2, 2) = c;
    return rotation;
}

Matrix4 Matrix4::createRotationZ(float angle) {
    Matrix4 rotation = Matrix4();
    float c = std::cos(angle);
    float s = std::sin(angle);
    rotation(0, 0) = c;
    rotation(0, 1) = -s;
    rotation(1, 0) = s;
    rotation(1, 1) = c;
    return rotation;
}

Matrix4 Matrix4::createPerspective(float fov, float aspectRatio, float near, float far) {
    Matrix4 perspective = Matrix4();
    float f = 1.0f / std::tan(fov / 2.0f);
    perspective(0, 0) = f / aspectRatio;
    perspective(1, 1) = f;
    perspective(2, 2) = (far + near) / (near - far);
    perspective(2, 3) = (2 * far * near) / (near - far);
    perspective(3, 2) = -1;
    perspective(3, 3) = 0;
    return perspective;
}

Matrix4 Matrix4::createOrthographic(float left, float right, float bottom, float top, float near, float far) {
    Matrix4 orthographic = Matrix4();
    orthographic(0, 0) = 2 / (right - left);
    orthographic(1, 1) = 2 / (top - bottom);
    orthographic(2, 2) = -2 / (far - near);
    orthographic(0, 3) = -(right + left) / (right - left);
    orthographic(1, 3) = -(top + bottom) / (top - bottom);
    orthographic(2, 3) = -(far + near) / (far - near);
    return orthographic;
}