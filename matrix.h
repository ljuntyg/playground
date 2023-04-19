#ifndef MATRIX_H
#define MATRIX_H

#include <vector>
#include <stdexcept>
#include <iostream>

class Vertex; // Forward declaration of Vertex

class Matrix {
public:
    Matrix(int rows, int cols);

    double& operator()(int row, int col);
    const double& operator()(int row, int col) const;
    Matrix operator*(const Matrix& other) const;
    Vertex operator*(const Vertex& vertex) const;

    static Matrix createTranslation(double x, double y, double z);
    static Matrix createScale(double x, double y, double z);
    static Matrix createRotationX(double angle);
    static Matrix createRotationY(double angle);
    static Matrix createRotationZ(double angle);
    static Matrix createPerspectiveProjection(double fov, double aspectRatio, double near, double far);

    int rows() const;
    int cols() const;
    void print() const;

private:
    int m_rows;
    int m_cols;
    std::vector<double> m_data;
};

#endif // MATRIX_H
