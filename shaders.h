#pragma once

#include <glad/glad.h>

namespace shaders
{
    GLuint compileShader(const GLenum type, const GLchar* source);
    GLuint createShaderProgram(const GLchar* vertexShaderSource, const GLchar* fragmentShaderSource);

    extern const GLchar* rendererVertexShaderSource;
    extern const GLchar* rendererFragmentShaderSource;
    extern const GLchar* skyboxVertexShaderSource;
    extern const GLchar* skyboxFragmentShaderSource;
    extern const GLchar* guiVertexShaderSource;
    extern const GLchar* guiFragmentShaderSource;
    extern const GLchar* textVertexShaderSource;
    extern const GLchar* textFragmentShaderSource;
}