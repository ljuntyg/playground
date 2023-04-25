#include <SDL2/SDL.h>
#include <iostream>
#include <Eigen>
#include <cmath>
#include <filesystem>

#include "renderer.h"

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("Playground - FPS: ", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, Renderer::WINDOW_WIDTH, Renderer::WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    Uint32 nextFrameTicks = SDL_GetTicks();

    std::vector<Triangle> cube = {
        // SOUTH
        Triangle(Eigen::Vector4d(0, 0, 0, 1), Eigen::Vector4d(0, 1, 0, 1), Eigen::Vector4d(1, 1, 0, 1)),
        Triangle(Eigen::Vector4d(0, 0, 0, 1), Eigen::Vector4d(1, 1, 0, 1), Eigen::Vector4d(1, 0, 0, 1)),

        // EAST
        Triangle(Eigen::Vector4d(1, 0, 0, 1), Eigen::Vector4d(1, 1, 0, 1), Eigen::Vector4d(1, 1, 1, 1)),
        Triangle(Eigen::Vector4d(1, 0, 0, 1), Eigen::Vector4d(1, 1, 1, 1), Eigen::Vector4d(1, 0, 1, 1)),

        // NORTH
        Triangle(Eigen::Vector4d(1, 0, 1, 1), Eigen::Vector4d(1, 1, 1, 1), Eigen::Vector4d(0, 1, 1, 1)),
        Triangle(Eigen::Vector4d(1, 0, 1, 1), Eigen::Vector4d(0, 1, 1, 1), Eigen::Vector4d(0, 0, 1, 1)),

        // WEST
        Triangle(Eigen::Vector4d(0, 0, 1, 1), Eigen::Vector4d(0, 1, 1, 1), Eigen::Vector4d(0, 1, 0, 1)),
        Triangle(Eigen::Vector4d(0, 0, 1, 1), Eigen::Vector4d(0, 1, 0, 1), Eigen::Vector4d(0, 0, 0, 1)),

        // TOP
        Triangle(Eigen::Vector4d(0, 1, 0, 1), Eigen::Vector4d(0, 1, 1, 1), Eigen::Vector4d(1, 1, 1, 1)),
        Triangle(Eigen::Vector4d(0, 1, 0, 1), Eigen::Vector4d(1, 1, 1, 1), Eigen::Vector4d(1, 1, 0, 1)),

        // BOTTOM
        Triangle(Eigen::Vector4d(1, 0, 1, 1), Eigen::Vector4d(0, 0, 1, 1), Eigen::Vector4d(0, 0, 0, 1)),
        Triangle(Eigen::Vector4d(1, 0, 1, 1), Eigen::Vector4d(0, 0, 0, 1), Eigen::Vector4d(1, 0, 0, 1)),
    };

    std::vector<Triangle> teapot = Renderer::loadObj("res/teapot.obj");

    std::vector<Triangle> gourd = Renderer::loadObj("res/gourd.obj");

    std::vector<Triangle> axis = Renderer::loadObj("res/axis.obj");

    std::vector<Triangle> shuttle = Renderer::loadObj("res/shuttle.obj");

    double rotX = 0;
    double rotY = 0;
    double rotZ = 0;
    double objectRotationSpeed = 0.01;

    SDL_Event e;
    bool quit = false;
    while (!quit) {

        Uint32 currentTicks = SDL_GetTicks();
        if (currentTicks >= nextFrameTicks) {
            // Update the next frame ticks
            nextFrameTicks = currentTicks + Renderer::TICKS_PER_FRAME;
            Renderer::frameCounter++;

            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    quit = true;
                } else if (e.type == SDL_MOUSEMOTION) {
                    // Check if the left mouse button is pressed
                    if (e.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
                        int mouseX = e.motion.xrel;
                        int mouseY = e.motion.yrel;

                        double dx = 0;
                        double dy = 0;

                        dx += mouseX * Renderer::mouseSensitivity;
                        dy -= mouseY * Renderer::mouseSensitivity;

                        Renderer::onYawPitch(dx, dy);
                    }
                }
            }

            Renderer::onYawPitch(0, 0); // Idk, need to call or weird perspective snap

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
                double dx = 0;
                dx += Renderer::rotationSpeed;
                Renderer::onYawPitch(dx, 0);
            }
            // Yaw left
            if (key_state[SDL_SCANCODE_LEFT]) {
                double dx = 0;
                dx -= Renderer::rotationSpeed;
                Renderer::onYawPitch(dx, 0);
            }
            // Pitch up
            if (key_state[SDL_SCANCODE_UP]) {
                double dy = 0;
                dy += Renderer::rotationSpeed;

                Renderer::onYawPitch(0, dy);
            }
            // Pitch down
            if (key_state[SDL_SCANCODE_DOWN]) {
                double dy = 0;
                dy -= Renderer::rotationSpeed;

                Renderer::onYawPitch(0, dy);
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

            // Calculate and display FPS every second
            if (currentTicks - Renderer::fpsUpdateTime >= 1000) {
                Renderer::fps = Renderer::frameCounter;
                Renderer::frameCounter = 0;
                Renderer::fpsUpdateTime = currentTicks;

                // Update the window title with the current FPS
                std::string windowTitle = "Playground - FPS: " + std::to_string(Renderer::fps);
                SDL_SetWindowTitle(window, windowTitle.c_str());
            }
        } else {
            // Sleep to prevent high CPU usage
            SDL_Delay(nextFrameTicks - currentTicks);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
