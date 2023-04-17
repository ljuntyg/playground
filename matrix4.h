#ifndef MATRIX4_H
#define MATRIX4_H

#include <array>

class Matrix4 {
public:
    // Constructors
    Matrix4(); // Initialize to identity matrix
    Matrix4(const std::array<float, 16>& data);

    // Matrix multiplication
    Matrix4 operator*(const Matrix4& other) const;

    // Accessors
    float operator()(int row, int col) const;
    float& operator()(int row, int col);

    // Transformation matrices
    static Matrix4 createTranslation(float x, float y, float z);
    static Matrix4 createScale(float x, float y, float z);
    static Matrix4 createRotationX(float angle);
    static Matrix4 createRotationY(float angle);
    static Matrix4 createRotationZ(float angle);

    // Projection matrices
    static Matrix4 createPerspective(float fov, float aspectRatio, float near, float far);
    static Matrix4 createOrthographic(float left, float right, float bottom, float top, float near, float far);

private:
    std::array<float, 16> m_data;
};

#endif // MATRIX4_H