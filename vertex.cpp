#include <cmath>

#include "vertex.h"

Vertex::Vertex(double x, double y, double z, double w) : Matrix(4, 1) {
    (*this)(0, 0) = x;
    (*this)(1, 0) = y;
    (*this)(2, 0) = z;
    (*this)(3, 0) = w;
}

Vertex Vertex::operator*(double scalar) const {
    return Vertex(x() * scalar, y() * scalar, z() * scalar, w() * scalar);
}

Vertex Vertex::operator/(double scalar) const {
    if (scalar == 0) {
        throw std::invalid_argument("Division by zero is not allowed.");
    }
    return Vertex(x() / scalar, y() / scalar, z() / scalar, w() / scalar);
}

Vertex Vertex::operator+(const Vertex& other) const {
    return Vertex(x() + other.x(), y() + other.y(), z() + other.z(), w() + other.w());
}

Vertex Vertex::operator-(const Vertex& other) const {
    return Vertex(x() - other.x(), y() - other.y(), z() - other.z(), w() + other.w());
}

Vertex& Vertex::operator+=(const Vertex& other) {
    setX(x() + other.x());
    setY(y() + other.y());
    setZ(z() + other.z());
    setW(w() + other.w());
    return *this;
}

Vertex& Vertex::operator-=(const Vertex& other) {
    setX(x() - other.x());
    setY(y() - other.y());
    setZ(z() - other.z());
    setW(w() - other.w());
    return *this;
}

double Vertex::dot(const Vertex& other) const {
    return x() * other.x() + y() * other.y() + z() * other.z() + w() * other.w();
}

Vertex Vertex::cross(const Vertex& other) const {
    return Vertex(
        y() * other.z() - z() * other.y(),
        z() * other.x() - x() * other.z(),
        x() * other.y() - y() * other.x(),
        0
    );
}

double Vertex::length() const {
    return std::sqrt(x() * x() + y() * y() + z() * z());
}

Vertex Vertex::normalize() const {
    double len = length();
    if (len == 0) {
        throw std::invalid_argument("Cannot normalize a zero-length vector.");
    }
    return Vertex(x() / len, y() / len, z() / len, 1);
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