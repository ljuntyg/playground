#include "triangle.h"

Triangle::Triangle(const Eigen::Vector4d& v1, const Eigen::Vector4d& v2, const Eigen::Vector4d& v3)
    : m_v1(v1), m_v2(v2), m_v3(v3) {}

const Eigen::Vector4d& Triangle::v1() const {
    return m_v1;
}

const Eigen::Vector4d& Triangle::v2() const {
    return m_v2;
}

const Eigen::Vector4d& Triangle::v3() const {
    return m_v3;
}

void Triangle::setV1(const Eigen::Vector4d& v) {
    m_v1 = v;
}

void Triangle::setV2(const Eigen::Vector4d& v) {
    m_v2 = v;
}

void Triangle::setV3(const Eigen::Vector4d& v) {
    m_v3 = v;
}