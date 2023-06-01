#pragma once

#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <unordered_map>

#include "obj_loader.h"

namespace renderer {
    enum RendererState
    {
        RENDERER_CREATED,
        RENDERER_CREATE_ERROR,
        RENDERER_RUN,
        RENDERER_PAUSE,
        renderStateCount
    };

    const std::unordered_map<std::string, glm::vec4> colorMap = {
        {"RED",     glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)},
        {"GREEN",   glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)},
        {"BLUE",    glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)},
        {"YELLOW",  glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)},
        {"CYAN",    glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)},
        {"MAGENTA", glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)},
        {"WHITE",   glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)},
        {"BLACK",   glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)}
    };

    int SDL_GLAD_init(SDL_Window* window, SDL_GLContext* context, float* WINDOW_WIDTH, float* WINDOW_HEIGHT);
    GLuint compileShader(const GLenum type, const GLchar *source);
    GLuint createShaderProgram(const GLchar *vertexShaderSource, const GLchar *fragmentShaderSource);

    struct Camera
    {
        glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
        glm::vec3 targetPos = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 lookDir = glm::vec3(0.0f, 0.0f, 1.0f);
        glm::vec3 lightPos = glm::vec3(0.0f, -1000.0f, -1000.0f);

        float cameraYaw = -M_PI_2;
        float cameraPitch = 0.0f;
        const float cameraSpeed = 1.0f;
        const float mouseSensitivity = 0.05f;
    };

    class Renderer 
    {
    public:
        RendererState RENDERER_STATE;

        Renderer();
        ~Renderer();

    private:
        void run();
        void drawObject();
        void handleInput(const SDL_Event* event, const Uint8* keyboardState);
        void onYawPitch(float dx, float dy);
        void onKeys(const int& key);

        std::vector<std::string> getObjFileNames(const std::string& folderName);
        std::vector<objl::Mesh> getTargetObjMesh();
        void nextTargetObj();

        Camera camera;
        std::string OBJ_PATH = "res/obj"; // Must be in working directory
        std::string targetFile = "mountains.obj"; // Must be in OBJ_PATH
        std::vector<std::string> allObjNames;
        std::vector<objl::Mesh> targetObj;

        SDL_Window* window;
        SDL_GLContext context;

        float WINDOW_WIDTH;
        float WINDOW_HEIGHT;
        const float NEAR_DIST = 0.1f;
        const float FAR_DIST = 10000.0f;
        const float FOV = M_PI_2;

        GLuint shaderProgram, VAO, VBO, EBO;
        const GLchar *rendererVertexShaderSource = ""; // Create
        const GLchar *rendererFragmentShaderSource = ""; // Create
    };
};