#include <cmath>

#include "matrix.h"
#include "vertex.h"

Matrix::Matrix(int rows, int cols) : m_rows(rows), m_cols(cols), m_data(rows * cols, 0) {}

double& Matrix::operator()(int row, int col) {
    return m_data[row * m_cols + col];
}

const double& Matrix::operator()(int row, int col) const {
    return m_data[row * m_cols + col];
}

Matrix Matrix::operator*(const Matrix& other) const {
    if (m_cols != other.m_rows) {
        throw std::invalid_argument("Incompatible matrices for multiplication");
    }

    Matrix result(m_rows, other.m_cols);
    for (int row = 0; row < m_rows; ++row) {
        for (int col = 0; col < other.m_cols; ++col) {
            for (int k = 0; k < m_cols; ++k) {
                result(row, col) += (*this)(row, k) * other(k, col);
            }
        }
    }

    return result;
}

Vertex Matrix::operator*(const Vertex& vertex) const {
    if (m_cols != 4) {
        throw std::invalid_argument("Incompatible matrix and vertex for multiplication");
    }

    Vertex result(0, 0, 0, 0);
    for (int row = 0; row < m_rows; row++) {
        for (int col = 0; col < 4; col++) {
            result(row, 0) += (*this)(row, col) * vertex(col, 0);
        }
    }

    if (result.w() != 0) {
        result.setX(result.x() / result.w());
        result.setY(result.y() / result.w());
        result.setZ(result.z() / result.w());
    }

    return result;
}

int Matrix::rows() const {
    return m_rows;
}

int Matrix::cols() const {
    return m_cols;
}

void Matrix::print() const {
    for (int i = 0; i < m_rows; i++) {
        for (int j = 0; j < m_cols; j++) {
            std::cout << (*this)(i, j) << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "\n" << std::endl;
}

Matrix Matrix::createTranslation(double x, double y, double z) {
    Matrix translation(4, 4);
    translation(0, 0) = 1;
    translation(1, 1) = 1;
    translation(2, 2) = 1;
    translation(3, 3) = 1;
    translation(0, 3) = x;
    translation(1, 3) = y;
    translation(2, 3) = z;
    return translation;
}

Matrix Matrix::createScale(double x, double y, double z) {
    Matrix scale(4, 4);
    scale(0, 0) = x;
    scale(1, 1) = y;
    scale(2, 2) = z;
    scale(3, 3) = 1;
    return scale;
}

Matrix Matrix::createRotationX(double angle) {
    Matrix rotation(4, 4);
    double c = std::cos(angle);
    double s = std::sin(angle);
    rotation(0, 0) = 1;
    rotation(1, 1) = c;
    rotation(1, 2) = -s;
    rotation(2, 1) = s;
    rotation(2, 2) = c;
    rotation(3, 3) = 1;
    return rotation;
}

Matrix Matrix::createRotationY(double angle) {
    Matrix rotation(4, 4);
    double c = std::cos(angle);
    double s = std::sin(angle);
    rotation(0, 0) = c;
    rotation(0, 2) = s;
    rotation(1, 1) = 1;
    rotation(2, 0) = -s;
    rotation(2, 2) = c;
    rotation(3, 3) = 1;
    return rotation;
}

Matrix Matrix::createRotationZ(double angle) {
    Matrix rotation(4, 4);
    double c = std::cos(angle);
    double s = std::sin(angle);
    rotation(0, 0) = c;
    rotation(0, 1) = -s;
    rotation(1, 0) = s;
    rotation(1, 1) = c;
    rotation(2, 2) = 1;
    rotation(3, 3) = 1;
    return rotation;
}

Matrix Matrix::createPerspectiveProjection(double fov, double aspectRatio, double near, double far) {
    Matrix projection(4, 4);
    double tan_half_fov = std::tan(fov / 2.0);
    double range = far - near;

    projection(0, 0) = 1.0 / (aspectRatio * tan_half_fov);
    projection(1, 1) = 1.0 / tan_half_fov;
    projection(2, 2) = -(far + near) / range;
    projection(2, 3) = -2 * far * near / range;
    projection(3, 2) = -1;
    projection(3, 3) = 0.0;

    return projection;
}


Matrix Matrix::createPointAt(const Vertex& position, const Vertex& target, const Vertex& up) {
    Matrix pointAt(4, 4);
    Vertex forward = (target - position).normalize();
    Vertex left = up.cross(forward).normalize();
    Vertex trueUp = forward.cross(left);

    pointAt(0, 0) = left.x();
    pointAt(0, 1) = left.y();
    pointAt(0, 2) = left.z();
    pointAt(0, 3) = 0;
    pointAt(1, 0) = trueUp.x();
    pointAt(1, 1) = trueUp.y();
    pointAt(1, 2) = trueUp.z();
    pointAt(1, 3) = 0;
    pointAt(2, 0) = forward.x();
    pointAt(2, 1) = forward.y();
    pointAt(2, 2) = forward.z();
    pointAt(2, 3) = 0;
    pointAt(3, 0) = position.x();
    pointAt(3, 1) = position.y();
    pointAt(3, 2) = position.z();
    pointAt(3, 3) = 1;

    return pointAt;
}

Matrix Matrix::inversedPointAt(const Matrix& pointAt) {
    Matrix inverse(4, 4);

    inverse(0, 0) = pointAt(0, 0);
    inverse(0, 1) = pointAt(1, 0);
    inverse(0, 2) = pointAt(2, 0);
    inverse(0, 3) = 0;
    inverse(1, 0) = pointAt(0, 1);
    inverse(1, 1) = pointAt(1, 1);
    inverse(1, 2) = pointAt(2, 1);
    inverse(1, 3) = 0;
    inverse(2, 0) = pointAt(0, 2);
    inverse(2, 1) = pointAt(1, 2);
    inverse(2, 2) = pointAt(2, 2);
    inverse(2, 3) = 0;
    inverse(3, 0) = -(pointAt(3, 0) * inverse(0, 0) + pointAt(3, 1) * inverse(1, 0) + pointAt(3, 2) * inverse(2, 0));
    inverse(3, 1) = -(pointAt(3, 0) * inverse(0, 1) + pointAt(3, 1) * inverse(1, 1) + pointAt(3, 2) * inverse(2, 1));
    inverse(3, 2) = -(pointAt(3, 0) * inverse(0, 2) + pointAt(3, 1) * inverse(1, 2) + pointAt(3, 2) * inverse(2, 2));
    inverse(3, 3) = 1;

    return inverse;
}
