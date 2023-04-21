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

    static constexpr double FAR = 10;
    static constexpr double NEAR = 1;

    static void drawObject(SDL_Renderer* renderer, std::vector<Vertex>& object, double rotX = 0, double rotY = 0, double rotZ = 0, double scale = 1);
    static void drawTriangle(SDL_Renderer* renderer, Triangle triangle);
    static std::vector<Vertex> loadObj(const std::string& filename);
};

#endif // RENDERER_H