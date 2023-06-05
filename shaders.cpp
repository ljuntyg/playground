#include "shaders.h"

#include <iostream>

namespace shaders
{
    // Function to compile a shader, helper method to createShaderProgram
    GLuint compileShader(const GLenum type, const GLchar *source)
    {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);

        GLint status;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if (!status)
        {
            GLchar infoLog[512];
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            std::cerr << "Failed to compile shader: " << infoLog << std::endl;
        }

        return shader;
    }

    // Function to create a shader program
    GLuint createShaderProgram(const GLchar* vertexShaderSource, const GLchar* fragmentShaderSource)
    {
        GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
        GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

        GLuint program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        GLint status;
        glGetProgramiv(program, GL_LINK_STATUS, &status);
        if (!status)
        {
            GLchar infoLog[512];
            glGetProgramInfoLog(program, 512, nullptr, infoLog);
            std::cerr << "Failed to link shader program: " << infoLog << std::endl;

            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);

            return 0;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return program;
    }

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

    const GLchar *guiVertexShaderSource = R"glsl(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec2 aTexCoords;

        out vec2 TexCoords;

        void main()
        {
            gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
            TexCoords = aTexCoords;
        }
    )glsl";
    const GLchar *guiFragmentShaderSource = R"glsl(
        #version 330 core
        out vec4 FragColor;

        in vec2 TexCoords;

        uniform sampler2D texture;

        void main()
        {
            FragColor = texture(texture, TexCoords);
        }
    )glsl";
}