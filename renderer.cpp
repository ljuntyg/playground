#include <iostream>
#include <algorithm>

#include "renderer.h"

namespace renderer
{
    Renderer::Renderer(float WINDOW_WIDTH, float WINDOW_HEIGHT) : WINDOW_WIDTH(WINDOW_WIDTH), WINDOW_HEIGHT(WINDOW_HEIGHT)
    {
        shaderProgram = createShaderProgram(rendererVertexShaderSource, rendererFragmentShaderSource);
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
    }

    Renderer::~Renderer() 
    {
        // Clean up shaderProgram, VAO, VBO, EBO
        glDeleteProgram(shaderProgram);
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

    void Renderer::drawObject(const std::vector<Mesh>& object, const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
    {
        for (const Mesh& mesh : object) 
        {
            glBindVertexArray(VAO);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), mesh.vertices.data(), GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), mesh.indices.data(), GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
            glEnableVertexAttribArray(0);

            // For Lighting
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, normal));
            glEnableVertexAttribArray(1);

            glBindVertexArray(0); // Unbind the VAO

            // Use the shader program
            glUseProgram(shaderProgram);

            // Pass the transformation and projection matrices to the shader program
            GLint modelLocation = glGetUniformLocation(shaderProgram, "model");
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));

            GLint viewLocation = glGetUniformLocation(shaderProgram, "view");
            glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));

            GLint projectionLocation = glGetUniformLocation(shaderProgram, "projection");
            glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

            // Pass the light and camera positions to the shader program
            GLint lightPosLocation = glGetUniformLocation(shaderProgram, "lightPos");
            glUniform3fv(lightPosLocation, 1, glm::value_ptr(lightPos));

            GLint cameraPosLocation = glGetUniformLocation(shaderProgram, "viewPos");
            glUniform3fv(cameraPosLocation, 1, glm::value_ptr(cameraPos));

            // Draw the mesh
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }
    }

    void Renderer::onYawPitch(float dx, float dy) 
    {
        // Update camera yaw and pitch
        cameraYaw += dx;
        cameraPitch += dy;

        // Clamp pitch between -89 and 89 degrees
        if (cameraPitch > glm::radians(89.0f)) 
        {
            cameraPitch = glm::radians(89.0f);
        }
        if (cameraPitch < glm::radians(-89.0f)) 
        {
            cameraPitch = glm::radians(-89.0f);
        }

        // Calculate the new look direction
        glm::vec3 newLookDir;
        newLookDir.x = cos(cameraYaw) * cos(cameraPitch);
        newLookDir.y = sin(cameraPitch);
        newLookDir.z = sin(cameraYaw) * cos(cameraPitch);
        lookDir = glm::normalize(newLookDir);

        // Update target position based on new look direction and camera position
        targetPos = cameraPos + lookDir;
    }

    void Renderer::onKeys(const int& key) 
    {
        glm::vec3 front = lookDir;
        glm::vec3 right = glm::normalize(glm::cross(cameraUp, front));
        glm::vec3 up = cameraUp;

        float sign = std::pow(-1, key);

        if (key == 0 || key == 1) // Forward/backward
        {
            front *= cameraSpeed;
            cameraPos += sign * front;
            targetPos += sign * front;
        }
        if (key == 2 || key == 3) // Right/left
        {
            right *= cameraSpeed;
            cameraPos += sign * right;
            targetPos += sign * right;
        }
        if (key == 4 || key == 5) // Up/down
        {
            up *= cameraSpeed;
            cameraPos += sign * up;
            targetPos += sign * up;
        }
    }

    std::vector<Mesh> Renderer::getTargetObj() 
    {
        objl::Loader loader;
        if (loader.LoadFile(OBJ_PATH + "/" + targetFile) == 0) 
        {
            std::cout << "Failed to load object file" << std::endl;
        } 
        else 
        {
            std::cout << "Successfully loaded object file" << std::endl;
        }
        return objlMeshToCustomMesh(loader.LoadedMeshes);
    }

    void Renderer::nextTargetObj()
    {
        std::vector<std::string> fileNames = allObjNames;

        // Remove file directory prefix from file names
        for (auto& file : fileNames) 
        {
            std::string prefix = OBJ_PATH + "\\";
            std::size_t found = file.find(prefix);
            
            if (found != std::string::npos) {
                file = file.substr(found + prefix.length());
            }
        }

        int objIx = std::find(fileNames.begin(), fileNames.end(), targetFile) - fileNames.begin();
        assert(objIx != fileNames.size()); // Means file name not found

        if (objIx == fileNames.size() - 1)
        {
            objIx = 0;
        }

        targetFile = fileNames[objIx+1];
        targetObj = getTargetObj();
    }

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
    GLuint createShaderProgram(const GLchar *vertexShaderSource, const GLchar *fragmentShaderSource)
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
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return program;
    }

    std::vector<std::string> getObjFiles(const std::string& folderName) 
    {
        std::vector<std::string> objFiles;

        try 
        {
            for (const auto& entry : std::filesystem::directory_iterator(folderName)) 
            {
                if (entry.is_regular_file() && entry.path().extension() == ".obj") 
                {
                    objFiles.push_back(entry.path().string());
                }
            }
        } 
        catch (const std::filesystem::filesystem_error& e) 
        {
            std::cerr << "Error: " << e.what() << std::endl;
        }

        return objFiles;
    }
}