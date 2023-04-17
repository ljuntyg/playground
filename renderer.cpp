#include "renderer.h"
#include <SDL2/SDL.h>
#include <iostream>

#include <cmath>
#include <vector>

void Renderer::drawTriangle(SDL_Renderer &renderer) {
    std::cout << "what";
}

void Renderer::drawPyramid(SDL_Renderer &renderer, const std::vector<Triangle>& triangles) {
    std::cout << "drawPyramid";
    for (const Triangle& triangle : triangles) {
        SDL_SetRenderDrawColor(&renderer, triangle.color.r, triangle.color.g, triangle.color.b, triangle.color.a);
        std::cout << "\ncolor set";
        SDL_RenderDrawLine(&renderer, triangle.v1.x, triangle.v1.y, triangle.v2.x, triangle.v2.y); //issue
        SDL_RenderDrawLine(&renderer, triangle.v2.x, triangle.v2.y, triangle.v3.x, triangle.v3.y);
        SDL_RenderDrawLine(&renderer, triangle.v3.x, triangle.v3.y, triangle.v1.x, triangle.v1.y);
    }
}
