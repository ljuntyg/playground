// Credit to Laurent NOËL (celeborn2bealive) for the gltf loader and the rendering 
// code and his tutorial at https://gltf-viewer-tutorial.gitlab.io/ along with the
// accompanying source code at https://gitlab.com/gltf-viewer-tutorial/gltf-viewer

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
#include <deque>

#include "input_state.h"
#include "pub_sub.h"
#include "tiny_gltf.h"
#include "util_gltf.h"
#include "gui.h"

namespace renderer 
{
    struct Camera
    {
        glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
        glm::vec3 targetPos = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 lookDir = glm::vec3(0.0f, 0.0f, 1.0f);

        float cameraYaw = 0.0f;
        float cameraPitch = 0.0f;
        float cameraSpeed = 1.0f;
        const float horizontalMouseSensitivity = 0.05f;
        const float verticalMouseSensitivity = 0.025f;
        const float smoothingFactor = 0.1f; // Lower value means more smoothing
    };

    struct CubeMap
    {
        GLuint textureID;
        std::filesystem::path path;
    };

    enum RendererState
    {
        RENDERER_CREATED,
        RENDERER_CREATE_ERROR,
        RENDERER_RUN,
        RENDERER_PAUSE,
        renderStateCount
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
        bool SDL_GLAD_init(SDL_Window** window, SDL_GLContext* context);
        bool initializeModel();
        bool initializeShaders(); 
        bool initializeCubemaps();
        bool initializeGUI();

        // Main event loop
        void run();
        // Update the camera's yaw and pitch to smoothly move towards the target yaw and pitch
        void onYawPitch(float targetYaw, float targetPitch, InputState* inputState);
        // Update the camera's position based on keyboard input
        void onKeys(const Uint8* keyboardState, InputState* inputState);

        // Draws each node of the model by calling drawNode
        void drawModel();
        // Recursively draw node and its children at nodeIdx, binds
        // materials for each primitive in the mesh of the node
        void drawNode(int nodeIdx, const glm::mat4 parentMatrix, bool isRoot = false);
        void bindMaterial(const int materialIndex);
        void drawSkybox();

        // Returns a vector of absolute paths to all folders with 6 .png files in folderName
        std::vector<std::filesystem::path> getCubemapPaths(std::string folderName);
        // Sets targetCubemapPath to the next one in allCubemapPaths, if the end is reached
        // it sets the cubemapPath to none and returns in order to render the model without 
        // a cubemap, the next call to nextTargetCubemap will set the cubemapPath to the first
        void nextTargetCubemap();

        // Returns a vector of absolute paths to all .glb and .gltf files in folderName
        std::vector<std::filesystem::path> getGLTFfilePaths(std::string folderName);
        // Sets targetGLTFpath to the next element in allGLTFpaths or the first 
        // element if the end is reached, then reinitializes the targetGLTFmodel
        void nextTargetGLTFmodel();
        
        Camera camera;
        InputState inputState;
        gui::GUIHandler* guiHandler;

        std::string CUBEMAPS_PATH = "res/cubemaps"; // Must be in working directory
        std::string MODELS_PATH = "res/models"; // Must be in working directory
        std::string targetCubemapFile = "DikhololoNight"; // Must be in CUBEMAPS_PATH, must match exact cubemap folder name
        std::string targetGLTFfile = "mount_fuji_volcano_wide_area_model_-_japan_4k.glb"; // Must be in MODELS_PATH, must match exact file name, only .glb or .gltf (only embedded .gltf files allowed) files allowed

        std::vector<std::filesystem::path> allCubemapPaths;
        std::filesystem::path targetCubemapPath;
        CubeMap targetCubemap;

        std::vector<std::filesystem::path> allGLTFpaths;
        std::filesystem::path targetGLTFpath;
        tinygltf::Model targetGLTFmodel;

        SDL_Window* window;
        SDL_GLContext context;

        bool running = true;
        float WINDOW_WIDTH;
        float WINDOW_HEIGHT;
        float NEAR_DIST; // Initialize using diagonal distance of the bounding box of the scene
        float FAR_DIST; // Initialize using diagonal distance of the bounding box of the scene
        const float FOV = (float)M_PI_2;
        const int LOGIC_FREQ_HZ = 0; // Set to 0 for 60Hz, not for capping FPS

        /* 
            MODEL SPECIFIC SHADER RELATED VARIABLES
         */
        GLint modelViewProjMatrixLoc, modelViewMatrixLoc, normalMatrixLoc, lightDirectionLoc, lightIntensityLoc,
            baseColorTextureLoc, baseColorFactorLoc, metallicRoughnessTextureLoc, metallicFactorLoc, roughnessFactorLoc,
            emissiveTextureLoc, emissiveFactorLoc, occlusionTextureLoc, occlusionStrengthLoc, applyOcclusionLoc;  
        GLuint shaderProgram, whiteTextureID = 0;
        std::vector<GLuint> modelTextureIDs, VAOs, VBOs;
        std::vector<utilgltf::VAOrange> meshToVertexArrays;
        glm::mat4 projMatrix, viewMatrix;
        // The diagonal distance of the bounding box produced by the model
        float sceneDiagonalDistance;
        float scaleFactor = 1.0f, luminanceFactor = 5.0f;
        // Azimuth and incline rotation of the light source in radians,
        // only applies if lightFromCamera (in drawModel) is set to false
        float lightAzimuth = (float)M_PI_4, lightIncline = (float)M_PI_4;

        /* 
            SKYBOX SPECIFIC SHADER RELATED VARIABLES
         */
        GLint viewSkyboxLoc, projectionSkyboxLoc;
        GLuint shaderProgramSkybox, skyboxVAO, skyboxVBO, skyboxEBO;
    };
};