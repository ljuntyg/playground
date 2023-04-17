#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include <vector>  // added vector header

struct Vertex {
    float x;
    float y;
    float z;

    Vertex() : x(0), y(0), z(0) {}
    Vertex(float x, float y, float z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }
};

struct Triangle {
    Vertex v1;
    Vertex v2;
    Vertex v3;
    SDL_Color color;

    Triangle() {
        color.r = 0;
        color.g = 0;
        color.b = 0;
        color.a = 255;
    }

    Triangle(Vertex v1, Vertex v2, Vertex v3, SDL_Color color) {
        this->v1 = v1;
        this->v2 = v2;
        this->v3 = v3;
        this->color = color;
    }
};

class Renderer {
public:
    static void drawTriangle(SDL_Renderer &renderer);
    static void drawPyramid(SDL_Renderer &renderer, const std::vector<Triangle>& triangles);
};

#endif // RENDERER_H
