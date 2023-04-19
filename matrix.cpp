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
    if (m_cols != vertex.rows()) {
        throw std::invalid_argument("Incompatible matrix and vertex for multiplication");
    }

    Matrix result(m_rows, 1);
    for (int row = 0; row < m_rows; ++row) {
        for (int col = 0; col < vertex.cols(); ++col) {
            for (int k = 0; k < m_cols; ++k) {
                result(row, col) += (*this)(row, k) * vertex(k, col);
            }
        }
    }

    return Vertex(result(0, 0), result(1, 0), result(2, 0), result(3, 0));
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

    projection(0, 0) = 1.0 / (aspectRatio * tan_half_fov);
    projection(1, 1) = 1.0 / tan_half_fov;
    projection(2, 2) = -(far + near) / (far - near);
    projection(2, 3) = -2.0 * far * near / (far - near);
    projection(3, 2) = -1.0;
    projection(3, 3) = 0.0;

    return projection;
}
