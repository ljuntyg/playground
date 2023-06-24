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
#include "input_state.h"

namespace renderer 
{
    enum RendererState
    {
        RENDERER_CREATED,
        RENDERER_CREATE_ERROR,
        RENDERER_RUN,
        RENDERER_PAUSE,
        renderStateCount
    };

    bool SDL_GLAD_init(SDL_Window** window, SDL_GLContext* context, float* WINDOW_WIDTH, float* WINDOW_HEIGHT);

    struct Camera
    {
        glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
        glm::vec3 targetPos = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 lookDir = glm::vec3(0.0f, 0.0f, 1.0f);

        float cameraYaw = (float)-M_PI_2;
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
        bool initializeObject();
        bool initializeShaders(); 
        void run();
        void drawObject();
        void onYawPitch(float dx, float dy, InputState* inputState);
        void onKeys(const Uint8* keyboardState, InputState* inputState);

        std::vector<std::string> getObjFilePaths(const std::string& folderName);
        std::vector<objl::Mesh> getTargetObjMeshes();
        void nextTargetObj();

        Camera camera;
        std::string OBJ_PATH = "res/obj"; // Must be in working directory
        std::string targetFile = "cessna.obj"; // Must be in OBJ_PATH
        std::vector<std::string> allObjPaths;
        std::vector<objl::Mesh> targetObj;

        SDL_Window* window;
        SDL_GLContext context;

        float WINDOW_WIDTH;
        float WINDOW_HEIGHT;
        const float NEAR_DIST = 0.1f;
        const float FAR_DIST = 10000.0f;
        const float FOV = (float)M_PI_2;
        const int LOGIC_FREQ_HZ = 0; // Set to 0 for 60Hz, not for capping FPS

        GLint modelLoc, viewLoc, projectionLoc, useTextureLoc, objectColorLoc;
        GLuint shaderProgram, VAO, VBO, EBO;
    };
};