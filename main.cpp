#include <SDL2/SDL.h>
#include <iostream>
#include <array>

#include "renderer.h"
#include "triangle.h"

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("My Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, Renderer::WINDOW_WIDTH, Renderer::WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    std::array<Triangle, 12> cube = {
        // SOUTH
        Triangle(Vertex(0, 0, 0, 1), Vertex(0, 1, 0, 1), Vertex(1, 1, 0, 1)),
        Triangle(Vertex(0, 0, 0, 1), Vertex(1, 1, 0, 1), Vertex(1, 0, 0, 1)),

        // EAST
        Triangle(Vertex(1, 0, 0, 1), Vertex(1, 1, 0, 1), Vertex(1, 1, 1, 1)),
        Triangle(Vertex(1, 0, 0, 1), Vertex(1, 1, 1, 1), Vertex(1, 0, 1, 1)),

        // NORTH
        Triangle(Vertex(1, 0, 1, 1), Vertex(1, 1, 1, 1), Vertex(0, 1, 1, 1)),
        Triangle(Vertex(1, 0, 1, 1), Vertex(0, 1, 1, 1), Vertex(0, 0, 1, 1)),

        // WEST
        Triangle(Vertex(0, 0, 1, 1), Vertex(0, 1, 1, 1), Vertex(0, 1, 0, 1)),
        Triangle(Vertex(0, 0, 1, 1), Vertex(0, 1, 0, 1), Vertex(0, 0, 0, 1)),

        // TOP
        Triangle(Vertex(0, 1, 0, 1), Vertex(0, 1, 1, 1), Vertex(1, 1, 1, 1)),
        Triangle(Vertex(0, 1, 0, 1), Vertex(1, 1, 1, 1), Vertex(1, 1, 0, 1)),

        // BOTTOM
        Triangle(Vertex(1, 0, 1, 1), Vertex(0, 0, 1, 1), Vertex(0, 0, 0, 1)),
        Triangle(Vertex(1, 0, 1, 1), Vertex(0, 0, 0, 1), Vertex(1, 0, 0, 1)),
    };

    double rotX = 0;
    double rotY = 0;
    double rotZ = 0;
    double rotation_speed = 0.0001;

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

        // Update rotation angles
        rotX += rotation_speed;
        rotY += rotation_speed;
        rotZ += rotation_speed;

        // Make calls to renderer here
        Renderer::testDraw(renderer, cube, rotX, rotY, rotZ);

        // Update the screen
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
