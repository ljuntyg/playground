#include "triangle.h"

Triangle::Triangle(const Vertex& v1, const Vertex& v2, const Vertex& v3)
    : m_v1(v1), m_v2(v2), m_v3(v3) {}

const Vertex& Triangle::v1() const {
    return m_v1;
}

const Vertex& Triangle::v2() const {
    return m_v2;
}

const Vertex& Triangle::v3() const {
    return m_v3;
}

void Triangle::setV1(const Vertex& v) {
    m_v1 = v;
}

void Triangle::setV2(const Vertex& v) {
    m_v2 = v;
}

void Triangle::setV3(const Vertex& v) {
    m_v3 = v;
}