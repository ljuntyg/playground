#include <SDL2/SDL.h>
#include "renderer.h"
#include "matrix4.h"

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("My Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    // Pyramid vertices
    Vertex apex(0, 1, 0);
    Vertex base1(-1, -1, -1);
    Vertex base2(1, -1, -1);
    Vertex base3(1, -1, 1);
    Vertex base4(-1, -1, 1);

    // Pyramid triangles
    SDL_Color color = {255, 255, 255, 255};
    std::vector<Triangle> triangles = {
        Triangle(apex, base1, base2, color),
        Triangle(apex, base2, base3, color),
        Triangle(apex, base3, base4, color),
        Triangle(apex, base4, base1, color),
        Triangle(base1, base2, base3, color),
        Triangle(base1, base3, base4, color)
    };

    Matrix4 model = Matrix4::createTranslation(0, 0, 5) * Matrix4::createRotationY(0.5);
    Matrix4 view = Matrix4::createTranslation(0, 0, -5);
    Matrix4 projection = Matrix4::createPerspective(M_PI_4, 640.0 / 480.0, 0.1, 100);
    Matrix4 mvp = projection * view * model;

    for (Triangle& triangle : triangles) {
        std::array<Vertex, 3> vertices = {triangle.v1, triangle.v2, triangle.v3};

        for (int i = 0; i < 3; ++i) {
            Vertex& vertex = vertices[i];
            std::array<float, 4> vec = {vertex.x, vertex.y, vertex.z, 1};
            std::array<float, 4> result = {0, 0, 0, 0};

            for (int row = 0; row < 4; ++row) {
                for (int col = 0; col < 4; ++col) {
                    result[row] += mvp(row, col) * vec[col];
                }
            }

            // Convert to homogeneous coordinates and map to screen space
            for (int j = 0; j < 3; ++j) {
                result[j] /= result[3];
            }
            
            vertex.x = ((result[0] + 1) / 2) * 640;
            vertex.y = ((1 - result[1]) / 2) * 480;
        }

        triangle.v1 = vertices[0];
        triangle.v2 = vertices[1];
        triangle.v3 = vertices[2];
    }

    SDL_Event e;
    bool quit = false;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        // Clear the renderer
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw the pyramid
        Renderer::drawPyramid(*renderer, triangles);

        // Update the screen
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
