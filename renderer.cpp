#include <SDL2/SDL.h>
#include <iostream>

#include "renderer.h"

void Renderer::testDraw(SDL_Renderer& renderer, std::array<Triangle, 12> cube) {
    for(auto triangle : cube) {
        std::cout << "triangle!";
    }
}