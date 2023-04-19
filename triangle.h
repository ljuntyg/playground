#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "vertex.h"

class Triangle {
public:
    Triangle(const Vertex& v1, const Vertex& v2, const Vertex& v3);

    const Vertex& v1() const;
    const Vertex& v2() const;
    const Vertex& v3() const;

    void setV1(const Vertex& v);
    void setV2(const Vertex& v);
    void setV3(const Vertex& v);

private:
    Vertex m_v1;
    Vertex m_v2;
    Vertex m_v3;
};

#endif // TRIANGLE_H
