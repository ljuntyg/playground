#include <SDL2/SDL.h>
#include <iostream>
#include <Eigen>
#include <cmath>
#include <filesystem>

#include "renderer.h"
#include "OBJ_Loader.h"

std::vector<Triangle> verticesToTriangles(const std::vector<objl::Vertex>& vertices) {
    std::vector<Triangle> triangles;
    size_t newSize = vertices.size() - (vertices.size() % 3); // Truncate the size to a multiple of 3

    for (size_t i = 0; i < newSize; i += 3) {
        Eigen::Vector4d v1(vertices[i].Position.X, vertices[i].Position.Y, vertices[i].Position.Z, 1.0);
        Eigen::Vector4d v2(vertices[i + 1].Position.X, vertices[i + 1].Position.Y, vertices[i + 1].Position.Z, 1.0);
        Eigen::Vector4d v3(vertices[i + 2].Position.X, vertices[i + 2].Position.Y, vertices[i + 2].Position.Z, 1.0);
        triangles.emplace_back(v1, v2, v3);
    }

    return triangles;
}

std::vector<Triangle> meshesToTriangles(const std::vector<objl::Mesh>& meshes) {
    std::vector<Triangle> triangles;

    for (const objl::Mesh& mesh : meshes) {
        for (size_t i = 0; i < mesh.Indices.size(); i += 3) {
            const objl::Vertex& v1 = mesh.Vertices[mesh.Indices[i]];
            const objl::Vertex& v2 = mesh.Vertices[mesh.Indices[i + 1]];
            const objl::Vertex& v3 = mesh.Vertices[mesh.Indices[i + 2]];

            Eigen::Vector4d ev1(v1.Position.X, v1.Position.Y, v1.Position.Z, 1.0);
            Eigen::Vector4d ev2(v2.Position.X, v2.Position.Y, v2.Position.Z, 1.0);
            Eigen::Vector4d ev3(v3.Position.X, v3.Position.Y, v3.Position.Z, 1.0);

            // You can customize the color based on the material properties if desired
            SDL_Color color = {100, 100, 100, 255};

            triangles.emplace_back(ev1, ev2, ev3, color);
        }
    }

    return triangles;
}

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

    objl::Loader loader;

    /* std::cout << "shop, success on 1, fail on 0: " << loader.LoadFile("res/butcher shop.obj") << std::endl;
    std::vector<Triangle> shop =  meshesToTriangles(loader.LoadedMeshes);

    std::cout << "vatican, success on 1, fail on 0: " << loader.LoadFile("res/civitas vaticana tosell.obj") << std::endl;
    std::vector<Triangle> vatican =  meshesToTriangles(loader.LoadedMeshes);

    std::cout << "kusatsu, success on 1, fail on 0: " << loader.LoadFile("res/kusatsu.obj") << std::endl;
    std::vector<Triangle> kusatsu =  meshesToTriangles(loader.LoadedMeshes);

    std::cout << "teapot, success on 1, fail on 0: " << loader.LoadFile("res/teapot.obj") << std::endl;
    std::vector<Triangle> teapot = meshesToTriangles(loader.LoadedMeshes);

    std::cout << "gourd, success on 1, fail on 0: " << loader.LoadFile("res/gourd.obj") << std::endl;
    std::vector<Triangle> gourd =  meshesToTriangles(loader.LoadedMeshes);

    std::cout << "axis, success on 1, fail on 0: " << loader.LoadFile("res/axis.obj") << std::endl;
    std::vector<Triangle> axis =  meshesToTriangles(loader.LoadedMeshes);

    std::cout << "shuttle, success on 1, fail on 0: " << loader.LoadFile("res/shuttle.obj") << std::endl;
    std::vector<Triangle> shuttle =  meshesToTriangles(loader.LoadedMeshes);

    std::cout << "city, success on 1, fail on 0: " << loader.LoadFile("res/3d_city_sample_terrain.obj") << std::endl;
    std::vector<Triangle> city =  meshesToTriangles(loader.LoadedMeshes);

    std::cout << "skycraper, success on 1, fail on 0: " << loader.LoadFile("res/skyscraper.obj") << std::endl;
    std::vector<Triangle> skycraper =  meshesToTriangles(loader.LoadedMeshes);

    std::cout << "cessna, success on 1, fail on 0: " << loader.LoadFile("res/cessna.obj") << std::endl;
    std::vector<Triangle> cessna =  meshesToTriangles(loader.LoadedMeshes); */

    std::cout << "mountains, success on 1, fail on 0: " << loader.LoadFile("res/mountains.obj") << std::endl;
    std::vector<Triangle> mountains =  meshesToTriangles(loader.LoadedMeshes);

    /* std::cout << "jianshazui, success on 1, fail on 0: " << loader.LoadFile("res/jjianshazui.obj") << std::endl;
    std::vector<Triangle> jianshazui =  meshesToTriangles(loader.LoadedMeshes);

    std::cout << "hongkongplace, success on 1, fail on 0: " << loader.LoadFile("res/hongkongplace.obj") << std::endl;
    std::vector<Triangle> hongkongplace =  meshesToTriangles(loader.LoadedMeshes);

    std::cout << "jordansouth, success on 1, fail on 0: " << loader.LoadFile("res/jordansouth.obj") << std::endl;
    std::vector<Triangle> jordansouth =  meshesToTriangles(loader.LoadedMeshes); */

    double rotX = 0;
    double rotY = 0;
    double rotZ = 0;
    double objectRotationSpeed = 0;

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
            Renderer::drawObject(renderer, mountains, rotX, rotY, rotZ);

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