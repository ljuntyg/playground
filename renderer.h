#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include <array>

#include "matrix.h"
#include "triangle.h"

class Renderer {
public:
    static const int WINDOW_WIDTH = 640;
    static const int WINDOW_HEIGHT = 480;

    static constexpr double FAR = 100;
    static constexpr double NEAR = 0.1;

    static void testDraw(SDL_Renderer* renderer, std::array<Triangle, 12> cube, double rotX, double rotY, double rotZ);
    static void drawTriangle(SDL_Renderer* renderer, Triangle triangle);
};

#endif // RENDERER_H