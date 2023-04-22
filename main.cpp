#include <SDL2/SDL.h>
#include <iostream>
#include <Eigen>
#include <cmath>

#include "renderer.h"
#include "triangle.h"

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("My Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, Renderer::WINDOW_WIDTH, Renderer::WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    Renderer::onYaw();

    std::vector<Eigen::Vector4d> cube = {
        // SOUTH
        Eigen::Vector4d(0, 0, 0, 1), Eigen::Vector4d(0, 1, 0, 1), Eigen::Vector4d(1, 1, 0, 1),
        Eigen::Vector4d(0, 0, 0, 1), Eigen::Vector4d(1, 1, 0, 1), Eigen::Vector4d(1, 0, 0, 1),

        // EAST
        Eigen::Vector4d(1, 0, 0, 1), Eigen::Vector4d(1, 1, 0, 1), Eigen::Vector4d(1, 1, 1, 1),
        Eigen::Vector4d(1, 0, 0, 1), Eigen::Vector4d(1, 1, 1, 1), Eigen::Vector4d(1, 0, 1, 1),

        // NORTH
        Eigen::Vector4d(1, 0, 1, 1), Eigen::Vector4d(1, 1, 1, 1), Eigen::Vector4d(0, 1, 1, 1),
        Eigen::Vector4d(1, 0, 1, 1), Eigen::Vector4d(0, 1, 1, 1), Eigen::Vector4d(0, 0, 1, 1),

        // WEST
        Eigen::Vector4d(0, 0, 1, 1), Eigen::Vector4d(0, 1, 1, 1), Eigen::Vector4d(0, 1, 0, 1),
        Eigen::Vector4d(0, 0, 1, 1), Eigen::Vector4d(0, 1, 0, 1), Eigen::Vector4d(0, 0, 0, 1),

        // TOP
        Eigen::Vector4d(0, 1, 0, 1), Eigen::Vector4d(0, 1, 1, 1), Eigen::Vector4d(1, 1, 1, 1),
        Eigen::Vector4d(0, 1, 0, 1), Eigen::Vector4d(1, 1, 1, 1), Eigen::Vector4d(1, 1, 0, 1),

        // BOTTOM
        Eigen::Vector4d(1, 0, 1, 1), Eigen::Vector4d(0, 0, 1, 1), Eigen::Vector4d(0, 0, 0, 1),
        Eigen::Vector4d(1, 0, 1, 1), Eigen::Vector4d(0, 0, 0, 1), Eigen::Vector4d(1, 0, 0, 1),
    };

    std::vector<Eigen::Vector4d> teapot = Renderer::loadObj("teapot.obj");

    std::vector<Eigen::Vector4d> gourd = Renderer::loadObj("gourd.obj");

    std::vector<Eigen::Vector4d> axis = Renderer::loadObj("axis.obj");

    double rotX = 0;
    double rotY = 0;
    double rotZ = 0;
    double objectRotationSpeed = 0.001;

    SDL_Event e;
    bool quit = false;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_MOUSEMOTION) {
                // Check if the left mouse button is pressed
                if (e.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
                    int mouseX = e.motion.xrel;
                    int mouseY = e.motion.yrel;

                    //Renderer::onMouse(mouseX, mouseY);
                }
            }
        }

        const Uint8* key_state = SDL_GetKeyboardState(NULL);

        // Move camera up
        if (key_state[SDL_SCANCODE_LSHIFT]) {
            Renderer::onKeys("UP");
        }
        // Move camera down
        if (key_state[SDL_SCANCODE_LCTRL]) {
            Renderer::onKeys("DOWN");
        }
        // Move camera right
        if (key_state[SDL_SCANCODE_A]) {
            Renderer::onKeys("RIGHT");
        }
        // Move camera left
        if (key_state[SDL_SCANCODE_D]) {
            Renderer::onKeys("LEFT");
        }
        // Move camera forward
        if (key_state[SDL_SCANCODE_W]) {
            Renderer::onKeys("FORWARD");
        }
        // Move camera backward
        if (key_state[SDL_SCANCODE_S]) {
            Renderer::onKeys("BACK");
        }
        // Yaw right
        if (key_state[SDL_SCANCODE_RIGHT]) {
            Renderer::cameraYaw += Renderer::rotationSpeed;
            Renderer::onYaw();
        }
        // Yaw left
        if (key_state[SDL_SCANCODE_LEFT]) {
            Renderer::cameraYaw -= Renderer::rotationSpeed;
            Renderer::onYaw();
        }

        // Set color to black, clear the renderer
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Update rotation angles
        rotX += objectRotationSpeed;
        rotY += objectRotationSpeed;
        rotZ += objectRotationSpeed;

        // Make calls to renderer here
        Renderer::drawObject(renderer, teapot, rotX, rotY, rotZ);

        // Update the screen
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
