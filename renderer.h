#pragma once

#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <unordered_map>

#include "structs.h"
#include "obj_loader.h"

namespace renderer 
{
    using namespace structs;

    enum RenderState
    {
        RENDERER_RUN,
        RENDERER_PAUSE,
        renderStateCount
    };

    GLuint compileShader(const GLenum type, const GLchar *source);
    GLuint createShaderProgram(const GLchar *vertexShaderSource, const GLchar *fragmentShaderSource);

    std::vector<std::string> getObjFiles(const std::string& folderName);

    class Renderer
    {
    public:
        Renderer(float WINDOW_WIDTH, float WINDOW_HEIGHT);
        ~Renderer();

        void drawObject(const std::vector<Mesh>& object, const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
        void onYawPitch(float dx, float dy);
        void onKeys(const int& key);

        std::vector<Mesh> getTargetObj();
        void nextTargetObj();

        RenderState RENDERER_STATE = RENDERER_RUN;;

        float WINDOW_WIDTH;
        float WINDOW_HEIGHT;
        const float NEAR_DIST = 0.1f;
        const float FAR_DIST = 10000.0f;
        const float FOV = M_PI_2;

        const std::string OBJ_PATH = "res/obj"; // Must be in working directory
        std::vector<std::string> allObjNames = getObjFiles(OBJ_PATH);
        std::string targetFile = "apartment building.obj";;
        std::vector<Mesh> targetObj = getTargetObj();

        glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
        glm::vec3 targetPos = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 lookDir = glm::vec3(0.0f, 0.0f, 1.0f);
        glm::vec3 lightPos = glm::vec3(0.0f, -1000.0f, -1000.0f);

        float cameraYaw = -M_PI_2; // -90 degrees
        float cameraPitch = 0.0f;
        const float cameraSpeed = 1.0f;
        const float mouseSensitivity = 0.05f;

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

        private:
            GLuint shaderProgram, VAO, VBO, EBO;

            const GLchar *rendererVertexShaderSource = R"glsl(
            #version 330 core
            layout (location = 0) in vec3 position;
            layout (location = 1) in vec3 normal; // Add this line

            uniform mat4 model;
            uniform mat4 view;
            uniform mat4 projection;

            out vec3 FragPos; // Add this line
            out vec3 Normal;  // Add this line

            void main()
            {
                gl_Position = projection * view * model * vec4(position, 1.0);
                FragPos = vec3(model * vec4(position, 1.0)); // Add this line
                Normal = mat3(transpose(inverse(model))) * normal; // Add this line
            }
        )glsl";
        const GLchar *rendererFragmentShaderSource = R"glsl(
            #version 330 core
            in vec3 FragPos;
            in vec3 Normal;

            out vec4 color;

            uniform vec3 lightPos;
            uniform vec3 viewPos;

            void main()
            {
                // Ambient
                float ambientStrength = 0.1;
                vec3 ambient = ambientStrength * vec3(1.0, 1.0, 1.0);

                // Diffuse
                vec3 norm = normalize(Normal);
                vec3 lightDir = normalize(lightPos - FragPos);
                float diff = max(dot(norm, lightDir), 0.0);
                vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);

                // Specular
                float specularStrength = 0.5;
                vec3 viewDir = normalize(viewPos - FragPos);
                vec3 reflectDir = reflect(lightDir, norm);
                float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
                vec3 specular = specularStrength * spec * vec3(1.0, 1.0, 1.0);

                // Final color
                vec3 result = (ambient + diffuse + specular) * vec3(1.0, 1.0, 1.0);
                color = vec4(result, 1.0);
            }
        )glsl";
    };
};