#include <SDL2/SDL.h>
#include <iostream>
#include <Eigen>
#include <cmath>
#include <filesystem>

#include "renderer.h"
#include "main.h"

std::vector<Mesh> Main::objlMeshToCustomMesh(const std::vector<objl::Mesh>& objlMeshes) {
    std::vector<Mesh> retVec;
    for (auto objlMesh : objlMeshes) {
        std::vector<Eigen::Vector4d> vertices;
        std::vector<size_t> indices;

        for (const auto& vertex : objlMesh.Vertices) {
            vertices.emplace_back(vertex.Position.X, vertex.Position.Y, vertex.Position.Z, 1.0);
        }

        for (const auto& index : objlMesh.Indices) {
            indices.push_back(index);
        }

        retVec.emplace_back(Mesh(vertices, indices));
    }
    return retVec;
}

std::vector<std::string> Main::getObjFiles(const std::string& folderName) {
    std::vector<std::string> objFiles;

    try {
        for (const auto& entry : std::filesystem::directory_iterator(folderName)) {
            if (entry.is_regular_file() && entry.path().extension() == ".obj") {
                objFiles.push_back(entry.path().string());
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return objFiles;
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("Playground - Frame Time: ", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, Renderer::WINDOW_WIDTH, Renderer::WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    Uint32 nextFrameTicks = SDL_GetTicks();

    std::vector<Eigen::Vector4d> cubeVertices = {
        // SOUTH
        Eigen::Vector4d(0, 0, 0, 1), Eigen::Vector4d(0, 1, 0, 1), Eigen::Vector4d(1, 1, 0, 1), Eigen::Vector4d(1, 0, 0, 1),
        // EAST
        Eigen::Vector4d(1, 0, 0, 1), Eigen::Vector4d(1, 1, 0, 1), Eigen::Vector4d(1, 1, 1, 1), Eigen::Vector4d(1, 0, 1, 1),
        // NORTH
        Eigen::Vector4d(1, 0, 1, 1), Eigen::Vector4d(1, 1, 1, 1), Eigen::Vector4d(0, 1, 1, 1), Eigen::Vector4d(0, 0, 1, 1),
        // WEST
        Eigen::Vector4d(0, 0, 1, 1), Eigen::Vector4d(0, 1, 1, 1), Eigen::Vector4d(0, 1, 0, 1), Eigen::Vector4d(0, 0, 0, 1),
        // TOP
        Eigen::Vector4d(0, 1, 0, 1), Eigen::Vector4d(0, 1, 1, 1), Eigen::Vector4d(1, 1, 1, 1), Eigen::Vector4d(1, 1, 0, 1),
        // BOTTOM
        Eigen::Vector4d(1, 0, 1, 1), Eigen::Vector4d(0, 0, 1, 1), Eigen::Vector4d(0, 0, 0, 1), Eigen::Vector4d(1, 0, 0, 1),
    };

    std::vector<size_t> cubeIndices = {
        // SOUTH
        0, 1, 2, 0, 2, 3,
        // EAST
        4, 5, 6, 4, 6, 7,
        // NORTH
        8, 9, 10, 8, 10, 11,
        // WEST
        12, 13, 14, 12, 14, 15,
        // TOP
        16, 17, 18, 16, 18, 19,
        // BOTTOM
        20, 21, 22, 20, 22, 23,
    };

    Mesh cubeMesh(cubeVertices, cubeIndices);
    std::vector<Mesh> cube = {cubeMesh};

    double rotX = 0;
    double rotY = 0;
    double rotZ = 0;
    double objectRotationSpeed = 0;

    bool tabKeyPressed = false;

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
            // Check TAB key for changing render mode
            if (key_state[SDL_SCANCODE_TAB]) {
                if (!tabKeyPressed) {
                    tabKeyPressed = true;

                    int currentMode = static_cast<int>(Renderer::RENDER_MODE);
                    currentMode++;

                    if (currentMode >= static_cast<int>(RenderModeCount)) {
                        currentMode = 0;
                    }

                    Renderer::RENDER_MODE = static_cast<RenderMode>(currentMode);
                }
            } else {
                tabKeyPressed = false;
            }

            // Set color to black, clear the renderer
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            // Update rotation angles
            rotX += objectRotationSpeed;
            rotY += objectRotationSpeed;
            rotZ += objectRotationSpeed;

            // Make calls to renderer here
            Renderer::drawObject(renderer, Renderer::targetObj, -1.514, rotY, rotZ);

            // Update the screen
            SDL_RenderPresent(renderer);

            // Calculate and display frame time every second
            if (currentTicks - Renderer::frameTimeUpdateTime >= 1000) {
                Renderer::frameTime = 1000.0f / Renderer::frameCounter;
                Renderer::frameCounter = 0;
                Renderer::frameTimeUpdateTime = currentTicks;

                // Update the window title with the current frame time
                std::string windowTitle = "Playground - Frame Time: " + std::to_string(Renderer::frameTime) + " ms";
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