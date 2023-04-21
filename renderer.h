#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include <array>
#include <Eigen>

#include "triangle.h"

class Renderer {
public:
    static const int WINDOW_WIDTH = 640;
    static const int WINDOW_HEIGHT = 480;

    static constexpr double FAR = 10;
    static constexpr double NEAR = 1;

    static constexpr double cameraSpeed = 0.1;
    static Eigen::Vector4d cameraPos;
    static Eigen::Vector4d targetPos;
    static Eigen::Vector4d upVector;

    static void drawObject(SDL_Renderer* renderer, std::vector<Eigen::Vector4d>& object, double rotX = 0, double rotY = 0, double rotZ = 0, double scale = 1);
    static void drawTriangle(SDL_Renderer* renderer, Triangle triangle);
    static std::vector<Eigen::Vector4d> loadObj(const std::string& filename);
};

#endif // RENDERER_H