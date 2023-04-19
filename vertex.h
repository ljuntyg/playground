#ifndef VERTEX_H
#define VERTEX_H

#include "matrix.h"

class Vertex : public Matrix {
public:
    Vertex(double x, double y, double z, double w);
    Vertex operator*(double scalar) const;
    Vertex operator/(double scalar) const;

    double x() const;
    double y() const;
    double z() const;
    double w() const;

    void setX(double x);
    void setY(double y);
    void setZ(double z);
    void setW(double w);
};

#endif // VERTEX_H
