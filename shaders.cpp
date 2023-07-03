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
        layout (location = 0) in vec3 aPos;

        uniform mat4 model;
        uniform mat4 projection;

        out vec2 vPos;

        void main()
        {
            gl_Position = projection * model * vec4(aPos, 1.0);
            vPos = aPos.xy;
        }
    )glsl";
    const GLchar *guiFragmentShaderSource = R"glsl(
        #version 330 core
        in vec2 vPos;

        out vec4 FragColor;

        uniform vec4 color;
        uniform vec2 resolution;  // Width and height of the UI element
        uniform float cornerRadius;  // Radius of the corners in pixels

        void main()
        {
            // Compute the position in the UI element in pixels
            vec2 pos = vPos * resolution;

            // Define the light source position at the top right corner
            vec2 lightSource = vec2(1.0, 1.0) * resolution;

            // Compute the distance from the light source to the current pixel
            float dist = distance(lightSource, pos);

            // Normalize the distance based on the size of the UI element,
            // so that it ranges from 0.0 at the light source to 1.0 at the corner
            float normDist = dist / (sqrt(2.0) * length(lightSource));

            vec4 startColor = vec4(color.rgb * 1.3, 1.0);
            vec4 endColor = vec4(color.rgb * 0.9, 1.0);

            // Create a smoother gradient by using smoothstep
            vec4 gradientColor = mix(startColor, endColor, smoothstep(0.0, 1.0, normDist));

            // Calculate distance to each corner and keep the minimum distance
            vec2 d = abs(pos - resolution*0.5) - (resolution*0.5 - vec2(cornerRadius));
            float distToCorner = length(max(d, 0.0));

            // Blend the color based on the distance to the corner
            FragColor = mix(gradientColor, vec4(0.0), smoothstep(0.0, 1.0, max(0.0, distToCorner) / cornerRadius));
        }
    )glsl";

    const GLchar* textVertexShaderSource = R"glsl(
        #version 330 core
        layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
        out vec2 TexCoords;

        uniform mat4 projection;
        uniform mat4 model;

        void main()
        {
            gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
            TexCoords = vertex.zw;
        }
    )glsl";
    const GLchar* textFragmentShaderSource = R"glsl(
        #version 330 core
        in vec2 TexCoords;
        out vec4 color;

        uniform sampler2D text;
        uniform vec4 textColor;
        
        const float smoothing = 1.0f;

        float median(float r, float g, float b) 
        {
            return max(min(r, g), min(max(r, g), b));
        }

        void main()
        {    
            vec3 sample = texture(text, TexCoords).rgb;
            float sigDist = median(sample.r, sample.g, sample.b) - 0.5;
            float alpha = clamp(sigDist/(fwidth(sigDist) * smoothing) + 0.5, 0.0, 1.0);
            color = vec4(textColor.rgb, textColor.a * alpha);
        }
    )glsl";
}