#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <Eigen>

class Triangle {
public:
    Triangle(const Eigen::Vector4d& v1, const Eigen::Vector4d& v2, const Eigen::Vector4d& v3);

    const Eigen::Vector4d& v1() const;
    const Eigen::Vector4d& v2() const;
    const Eigen::Vector4d& v3() const;

    void setV1(const Eigen::Vector4d& v);
    void setV2(const Eigen::Vector4d& v);
    void setV3(const Eigen::Vector4d& v);

private:
    Eigen::Vector4d m_v1;
    Eigen::Vector4d m_v2;
    Eigen::Vector4d m_v3;
};

#endif // TRIANGLE_H
