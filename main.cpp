#include <SDL2/SDL.h>
#include <iostream>
#include <array>

#include "renderer.h"

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("My Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, Renderer::WINDOW_WIDTH, Renderer::WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    std::array<Triangle, 12> cube = {Triangle(Vertex(0, 0, 0), Vertex(0, 1, 0), Vertex(1, 1, 0)), Triangle(Vertex(0, 0, 0), Vertex(1, 1, 0), Vertex(1, 0, 0)), // SOUTH
                                    Triangle(Vertex(1,0, 0), Vertex(1, 1, 0), Vertex(1, 1, 1)), Triangle(Vertex(1, 0, 0), Vertex(1, 1, 1), Vertex(1, 0, 1)), // EAST
                                    Triangle(Vertex(1, 0, 1), Vertex(1, 1, 1), Vertex(0, 1, 1)), Triangle(Vertex(1, 0, 1), Vertex(0, 1, 1), Vertex(0, 0, 1)), // NORTH
                                    Triangle(Vertex(0, 0, 1), Vertex(0, 1, 1), Vertex(0, 1, 0)), Triangle(Vertex(0, 0, 1), Vertex(0, 1, 0), Vertex(0, 0, 0)), // WEST
                                    Triangle(Vertex(0, 1, 0), Vertex(0, 1, 1), Vertex(1, 1, 1)), Triangle(Vertex(0, 1, 0), Vertex(1, 1, 1), Vertex(1, 1, 0)), // TOP
                                    Triangle(Vertex(1, 0, 1), Vertex(0, 0, 1), Vertex(0, 0, 0)), Triangle(Vertex(1, 0, 1), Vertex(0, 0, 0), Vertex(1, 0, 0))}; // BOTTOM

    SDL_Event e;
    bool quit = false;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }
        
        // Set color to black, clear the renderer
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Make calls to renderer here
        Renderer::testDraw(*renderer, cube);

        // Update the screen
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
