#include "renderer.h"
#include <SDL2/SDL.h>
#include <iostream>

void Renderer::drawTriangle(SDL_Renderer &renderer) {
    // Define the triangle's vertices
    int centerX = 320;
    int centerY = 240;
    int sideLength = 160;
    int halfSideLength = sideLength / 2;
    int height = static_cast<int>(sqrt(3) * halfSideLength / 2);

    SDL_Point vertices[3] = {
        {centerX - halfSideLength, centerY + height / 2},
        {centerX + halfSideLength, centerY + height / 2},
        {centerX, centerY - height / 2}
    };

    // Set the draw color to green
    SDL_SetRenderDrawColor(&renderer, 0, 255, 0, 255);

    // Draw the triangle
    SDL_RenderDrawLine(&renderer, vertices[0].x, vertices[0].y, vertices[1].x, vertices[1].y);
    SDL_RenderDrawLine(&renderer, vertices[1].x, vertices[1].y, vertices[2].x, vertices[2].y);
    SDL_RenderDrawLine(&renderer, vertices[2].x, vertices[2].y, vertices[0].x, vertices[0].y);
}
