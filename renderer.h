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

#include "structs.h"
#include "obj_loader.h"

namespace renderer 
{
    using namespace structs;

    const float WINDOW_WIDTH = 1280.0f;
    const float WINDOW_HEIGHT = 720.0f;
    const float NEAR_DIST = 0.1f;
    const float FAR_DIST = 10000.0f;
    const float FOV = M_PI_2;

    extern std::string objFolder;
    extern std::vector<std::string> allObjNames;
    extern std::string targetFile;
    extern std::vector<Mesh> targetObj;

    extern glm::vec3 cameraPos;
    extern glm::vec3 targetPos;
    extern glm::vec3 cameraUp;
    extern glm::vec3 lookDir;
    extern glm::vec3 lightPos;

    extern float cameraYaw;
    extern float cameraPitch;
    const float cameraSpeed = 1.0f;
    const float mouseSensitivity = 0.05f;

    const std::vector<Vertex> cubeVertices {
        // Front face
        Vertex(glm::vec3(-0.5, -0.5,  0.5), glm::vec3(0, 0, 1), glm::vec2(0, 0)),
        Vertex(glm::vec3( 0.5, -0.5,  0.5), glm::vec3(0, 0, 1), glm::vec2(1, 0)),
        Vertex(glm::vec3( 0.5,  0.5,  0.5), glm::vec3(0, 0, 1), glm::vec2(1, 1)),
        Vertex(glm::vec3(-0.5,  0.5,  0.5), glm::vec3(0, 0, 1), glm::vec2(0, 1)),
        // Back face
        Vertex(glm::vec3(-0.5, -0.5, -0.5), glm::vec3(0, 0, -1), glm::vec2(0, 0)),
        Vertex(glm::vec3( 0.5, -0.5, -0.5), glm::vec3(0, 0, -1), glm::vec2(1, 0)),
        Vertex(glm::vec3( 0.5,  0.5, -0.5), glm::vec3(0, 0, -1), glm::vec2(1, 1)),
        Vertex(glm::vec3(-0.5,  0.5, -0.5), glm::vec3(0, 0, -1), glm::vec2(0, 1)),
        // Left face
        Vertex(glm::vec3(-0.5, -0.5, -0.5), glm::vec3(-1, 0, 0), glm::vec2(0, 0)),
        Vertex(glm::vec3(-0.5, -0.5,  0.5), glm::vec3(-1, 0, 0), glm::vec2(1, 0)),
        Vertex(glm::vec3(-0.5,  0.5,  0.5), glm::vec3(-1, 0, 0), glm::vec2(1, 1)),
        Vertex(glm::vec3(-0.5,  0.5, -0.5), glm::vec3(-1, 0, 0), glm::vec2(0, 1)),
        // Right face
        Vertex(glm::vec3( 0.5, -0.5, -0.5), glm::vec3(1, 0, 0), glm::vec2(0, 0)),
        Vertex(glm::vec3( 0.5, -0.5,  0.5), glm::vec3(1, 0, 0), glm::vec2(1, 0)),
        Vertex(glm::vec3( 0.5,  0.5,  0.5), glm::vec3(1, 0, 0), glm::vec2(1, 1)),
        Vertex(glm::vec3( 0.5,  0.5, -0.5), glm::vec3(1, 0, 0), glm::vec2(0, 1)),
        // Bottom face
        Vertex(glm::vec3(-0.5, -0.5, -0.5), glm::vec3(0, -1, 0), glm::vec2(0, 0)),
        Vertex(glm::vec3( 0.5, -0.5, -0.5), glm::vec3(0, -1, 0), glm::vec2(1, 0)),
        Vertex(glm::vec3( 0.5, -0.5,  0.5), glm::vec3(0, -1, 0), glm::vec2(1, 1)),
        Vertex(glm::vec3(-0.5, -0.5,  0.5), glm::vec3(0, -1, 0), glm::vec2(0, 1)),
        // Top face
        Vertex(glm::vec3(-0.5,  0.5, -0.5), glm::vec3(0, 1, 0), glm::vec2(0, 0)),
        Vertex(glm::vec3( 0.5,  0.5, -0.5), glm::vec3(0, 1, 0), glm::vec2(1, 0)),
        Vertex(glm::vec3( 0.5,  0.5,  0.5), glm::vec3(0, 1, 0), glm::vec2(1, 1)),
        Vertex(glm::vec3(-0.5,  0.5,  0.5), glm::vec3(0, 1, 0), glm::vec2(0, 1))
    };
    const std::vector<unsigned int> cubeIndices {
        // Front face
        0, 1, 2, 0, 2, 3,
        // Back face
        4, 5, 6, 4, 6, 7,
        // Left face
        8, 9, 10, 8, 10, 11,
        // Right face
        12, 13, 14, 12, 14, 15,
        // Bottom face
        16, 17, 18, 16, 18, 19,
        // Top face
        20, 21, 22, 20, 22, 23
    };
    extern Mesh cube;

    extern const GLchar *vertexShaderSource;
    extern const GLchar *fragmentShaderSource;
    
    GLuint compileShader(const GLenum type, const GLchar *source);
    GLuint createShaderProgram(const GLchar *vertexShaderSource, const GLchar *fragmentShaderSource);

    void drawObject(const std::vector<Mesh>& object, const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
    void onYawPitch(float dx, float dy);
    void onKeys(const int& key);

    std::vector<std::string> getObjFiles(const std::string& folderName);
    std::vector<Mesh> getTargetObj();
};