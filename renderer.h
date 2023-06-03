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

    bool SDL_GLAD_init(SDL_Window** window, SDL_GLContext* context, float* WINDOW_WIDTH, float* WINDOW_HEIGHT);
    GLuint compileShader(const GLenum type, const GLchar *source);
    GLuint createShaderProgram(const GLchar* vertexShaderSource, const GLchar* fragmentShaderSource);

    struct Camera
    {
        glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
        glm::vec3 targetPos = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 lookDir = glm::vec3(0.0f, 0.0f, 1.0f);
        glm::vec3 lightPos = glm::vec3(0.0f, -1000.0f, -1000.0f);

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
        void onYawPitch(const SDL_Event* event);
        void onKeys(const Uint8* keyboardState);

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

        GLint modelLoc, viewLoc, projectionLoc, useTextureLoc, objectColorLoc;
        GLuint shaderProgram, VAO, VBO, EBO;
        const GLchar *rendererVertexShaderSource = R"glsl(
            #version 330 core
            layout (location = 0) in vec3 aPos;
            layout (location = 1) in vec3 aNormal;
            layout (location = 2) in vec2 aTexCoords;

            out vec2 TexCoords;

            uniform mat4 model;
            uniform mat4 view;
            uniform mat4 projection;

            void main()
            {
                gl_Position = projection * view * model * vec4(aPos, 1.0);
                TexCoords = aTexCoords;
            }
        )glsl";

        const GLchar *rendererFragmentShaderSource = R"glsl(
            #version 330 core
            out vec4 FragColor;

            in vec2 TexCoords;

            uniform sampler2D texture_diffuse1;
            uniform vec3 objectColor;
            uniform bool useTexture;

            void main()
            {    
                if (useTexture)
                {
                    FragColor = texture(texture_diffuse1, TexCoords);
                }
                else
                {
                    FragColor = vec4(objectColor, 1.0);
                }
            }
        )glsl";
    };
};