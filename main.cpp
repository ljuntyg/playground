#include <SDL2/SDL.h>
#include <iostream>
#include <array>

#include "renderer.h"
#include "triangle.h"

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("My Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, Renderer::WINDOW_WIDTH, Renderer::WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    std::vector<Vertex> cube = {
        // SOUTH
        Vertex(0, 0, 0, 1), Vertex(0, 1, 0, 1), Vertex(1, 1, 0, 1),
        Vertex(0, 0, 0, 1), Vertex(1, 1, 0, 1), Vertex(1, 0, 0, 1),

        // EAST
        Vertex(1, 0, 0, 1), Vertex(1, 1, 0, 1), Vertex(1, 1, 1, 1),
        Vertex(1, 0, 0, 1), Vertex(1, 1, 1, 1), Vertex(1, 0, 1, 1),

        // NORTH
        Vertex(1, 0, 1, 1), Vertex(1, 1, 1, 1), Vertex(0, 1, 1, 1),
        Vertex(1, 0, 1, 1), Vertex(0, 1, 1, 1), Vertex(0, 0, 1, 1),

        // WEST
        Vertex(0, 0, 1, 1), Vertex(0, 1, 1, 1), Vertex(0, 1, 0, 1),
        Vertex(0, 0, 1, 1), Vertex(0, 1, 0, 1), Vertex(0, 0, 0, 1),

        // TOP
        Vertex(0, 1, 0, 1), Vertex(0, 1, 1, 1), Vertex(1, 1, 1, 1),
        Vertex(0, 1, 0, 1), Vertex(1, 1, 1, 1), Vertex(1, 1, 0, 1),

        // BOTTOM
        Vertex(1, 0, 1, 1), Vertex(0, 0, 1, 1), Vertex(0, 0, 0, 1),
        Vertex(1, 0, 1, 1), Vertex(0, 0, 0, 1), Vertex(1, 0, 0, 1),
    };

    std::vector<Vertex> teapot = Renderer::loadObj("teapot.obj");

    std::vector<Vertex> gourd = Renderer::loadObj("gourd.obj");

    Matrix testMatrix(4, 4);
    testMatrix(0, 0) = 2;
    testMatrix(0, 1) = 3;
    testMatrix(0, 2) = 4;
    testMatrix(1, 1) = 1;
    testMatrix(1, 2) = 4;
    testMatrix(2, 0) = 3;
    testMatrix(3, 0) = 1;
    testMatrix(3, 2) = 1;

    Vertex testVertex(3, 2, 1, 1);

    (testMatrix * testVertex).print();

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

        const Uint8* key_state = SDL_GetKeyboardState(NULL);
            
        // Move camera up
        if (key_state[SDL_SCANCODE_LSHIFT]) {
            std::cout << "implement";
        }
        
        // Move camera down
        if (key_state[SDL_SCANCODE_LCTRL]) {
            std::cout << "implement";
        }

        // Move camera left
        if (key_state[SDL_SCANCODE_A]) {
            std::cout << "implement";
        }

        // Move camera right
        if (key_state[SDL_SCANCODE_D]) {
            std::cout << "implement";
        }

        // Move camera forward
        if (key_state[SDL_SCANCODE_W]) {
            std::cout << "implement";
        }

        // Move camera backward
        if (key_state[SDL_SCANCODE_S]) {
            std::cout << "implement";
        }

        // Set color to black, clear the renderer
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Update rotation angles
        rotX += rotation_speed;
        rotY += rotation_speed;
        rotZ += rotation_speed;

        // Make calls to renderer here
        Renderer::drawObject(renderer, cube, rotX, rotY, rotZ, 1);

        // Update the screen
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
