#include "vertex.h"

Vertex::Vertex(double x, double y, double z, double w = 1) : Matrix(4, 1) {
    (*this)(0, 0) = x;
    (*this)(1, 0) = y;
    (*this)(2, 0) = z;
    (*this)(3, 0) = 1;
}

Vertex Vertex::operator*(double scalar) const {
    return Vertex(x() * scalar, y() * scalar, z() * scalar);
}

Vertex Vertex::operator/(double scalar) const {
    if (scalar == 0) {
        throw std::invalid_argument("Division by zero is not allowed.");
    }
    return Vertex(x() / scalar, y() / scalar, z() / scalar);
}

double Vertex::x() const {
    return (*this)(0, 0);
}

double Vertex::y() const {
    return (*this)(1, 0);
}

double Vertex::z() const {
    return (*this)(2, 0);
}

double Vertex::w() const {
    return (*this)(3, 0);
}

void Vertex::setX(double x) {
    (*this)(0, 0) = x;
}

void Vertex::setY(double y) {
    (*this)(1, 0) = y;
}

void Vertex::setZ(double z) {
    (*this)(2, 0) = z;
}

void Vertex::setW(double w) {
    (*this)(3, 0) = w;
}