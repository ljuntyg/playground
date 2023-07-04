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
#include <array>

#include "obj_loader.h"
#include "input_state.h"
#include "pub_sub.h"

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

    // TODO: Not sure if appropriate struct for a cubemap
    struct CubeMap
    {
        GLuint textureID;
        std::filesystem::path path;
    };

    class Renderer : public Subscriber, public Publisher
    {
    public:
        RendererState RENDERER_STATE;

        Renderer();
        ~Renderer();

        void notify(const event::Event* event) override;
        void quit();

    private:
        bool initializeObject();
        bool initializeShaders(); 
        bool initializeCubemaps();
        void run();
        void drawObject();
        void drawSkybox();
        void onYawPitch(float dx, float dy, InputState* inputState);
        void onKeys(const Uint8* keyboardState, InputState* inputState);

        std::vector<std::string> getObjFilePaths(std::string folderName);
        std::vector<objl::Mesh> getTargetObjMeshes();
        void nextTargetObj();

        std::vector<std::filesystem::path> getCubemapPaths(std::string folderName);

        Camera camera;
        std::string CUBEMAPS_PATH = "res/cubemaps"; // Must be in working directory
        std::string OBJ_PATH = "res/obj"; // Must be in working directory
        std::string targetCubemapFile = "Skybox1"; // Must be in CUBEMAPS_PATH, must match exact file name
        std::string targetObjFile = "cessna.obj"; // Must be in OBJ_PATH, must match exact file name
        std::vector<std::string> allObjPaths;
        std::vector<objl::Mesh> targetObj;
        std::vector<CubeMap*> cubemaps;
        CubeMap* targetCubemap = nullptr;

        SDL_Window* window;
        SDL_GLContext context;

        bool running = true;
        float WINDOW_WIDTH;
        float WINDOW_HEIGHT;
        const float NEAR_DIST = 1.0f;
        const float FAR_DIST = 10000.0f;
        const float FOV = (float)M_PI_2;
        const int LOGIC_FREQ_HZ = 0; // Set to 0 for 60Hz, not for capping FPS

        // TODO: Add separate viewLoc and projectionLoc specific to skybox
        GLint modelLoc, viewLoc, projectionLoc, useTextureLoc, objectColorLoc;
        GLuint shaderProgram, VAO, VBO, EBO;
        GLuint shaderProgramSkybox, skyboxVAO, skyboxVBO, skyboxEBO;
    };
};