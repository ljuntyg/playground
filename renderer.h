#ifndef RENDERER_H
#define RENDERER_H

#include <array>

struct Vertex {
    double x;
    double y;
    double z;

    Vertex(double x, double y, double z) : x(x), y(y), z(z) {}
};

struct Triangle {
    Vertex a;
    Vertex b;
    Vertex c;

    Triangle(Vertex a, Vertex b, Vertex c) : a(a), b(b), c(c) {}
};

class Renderer {
public:
    static const int WINDOW_WIDTH = 640;
    static const int WINDOW_HEIGHT = 480;

    static constexpr double FAR = 100;
    static constexpr double NEAR = 0.1;

    static void testDraw(SDL_Renderer& renderer, std::array<Triangle, 12> cube);
};

#endif // RENDERER_H