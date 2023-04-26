#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include <cmath>
#include <array>
#include <Eigen>

#include "structs.h"

class Renderer {
public:
    // Rendering constants, scalefactor for equal x,y-coordinate scaling in projection
    static const int WINDOW_WIDTH = 1280;
    static const int WINDOW_HEIGHT = 720;
    static constexpr double scaleFactor = std::min(WINDOW_WIDTH, WINDOW_HEIGHT) / 2.0;
    static const int TICKS_PER_FRAME = 1000 / 60;

    static constexpr double FAR = 1000;
    static constexpr double NEAR = 0.1;
    static constexpr double FOV = 2 * M_PI / 3;

    static constexpr double speedScalar = 2;
    static constexpr double mouseSensitivity = 0.01 * speedScalar;
    static constexpr double cameraSpeed = 0.1 * speedScalar;
    static constexpr double rotationSpeed = 0.01 * speedScalar;

    static std::vector<double> depthBuffer;
    static Uint32 frameCounter;
    static Uint32 fpsUpdateTime;
    static int fps;

    // Movement related attributes/methods
    static double cameraYaw;
    static double cameraPitch;

    static Eigen::Vector4d lookDir;
    static Eigen::Vector4d cameraPos;
    static Eigen::Vector4d targetPos;
    static Eigen::Vector4d lightDir;

    static void onKeys(const std::string& key);
    static void onYawPitch(const double& dx, const double& dy);

    // Drawing/object related methods
    static void drawObject(SDL_Renderer* renderer, std::vector<Triangle>& object, double rotX = 0, double rotY = 0, double rotZ = 0, double scale = 1);
    static void drawTriangle(SDL_Renderer* renderer, const Triangle& triangle);
    static void drawTriangle(SDL_Renderer* renderer, const Triangle& triangle, SDL_Color color);
    static Eigen::Vector4d linePlaneIntersection(const ClipPlane& clipPlane, const Eigen::Vector4d& lineStart, const Eigen::Vector4d& lineEnd);
    static int clipTriangleAgainstPlane(const ClipPlane& clipPlane, Triangle& in_tri, Triangle& out_tri1, Triangle& out_tri2);
    static SDL_Color getShadingColor(double intensity);
    static void visualizeDepthBuffer(SDL_Renderer* renderer);
};

#endif // RENDERER_H