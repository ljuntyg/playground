#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include <cmath>
#include <array>
#include <Eigen>

#include "triangle.h"

class Renderer {
public:
    static const int WINDOW_WIDTH = 680;
    static const int WINDOW_HEIGHT = 480;

    static constexpr double FAR = 100;
    static constexpr double NEAR = 0.1;
    static constexpr double FOV = M_PI / 2;

    static constexpr double mouseSensitivity = 0.01;
    static constexpr double cameraSpeed = 0.1;
    static constexpr double rotationSpeed = 0.01;

    // Movement related attributes/methods
    static double cameraYaw;
    static double cameraPitch;
    static Eigen::Vector4d lookDir;
    static Eigen::Vector4d cameraPos;
    static Eigen::Vector4d targetPos;
    static void onKeys(const std::string& key);
    static void onYaw();
    static void onPitch();
    static void onMouse();

    // Drawing/object related methods
    static void drawObject(SDL_Renderer* renderer, std::vector<Eigen::Vector4d>& object, double rotX = 0, double rotY = 0, double rotZ = 0, double scale = 1);
    static void drawTriangle(SDL_Renderer* renderer, Triangle triangle);
    static std::vector<Eigen::Vector4d> loadObj(const std::string& filename);
};

#endif // RENDERER_H