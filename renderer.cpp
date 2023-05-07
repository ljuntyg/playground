#include <iostream>

#include "renderer.h"

namespace renderer 
{
    std::string objFolder = "res"; // Must be in working directory
    std::vector<std::string> allObjNames = getObjFiles(objFolder);
    std::string targetFile = "apartment building.obj"; // Don't forget .obj extension
    std::vector<Mesh> targetObj = getTargetObj();

    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
    glm::vec3 targetPos = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 lookDir = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 lightPos = glm::vec3(0.0f, 500.0f, -500.0f);

    float cameraYaw = -M_PI_2; // -90 degrees
    float cameraPitch = 0.0f;
    Mesh cube(cubeVertices, cubeIndices);

    const GLchar *vertexShaderSource = R"glsl(
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
    const GLchar *fragmentShaderSource = R"glsl(
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

    // Function to compile a shader
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

    void drawObject(const std::vector<Mesh>& object, const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) 
    {
        GLuint shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);

        // Set up vertex array and buffer objects
        GLuint VAO, VBO, EBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        // Loop through the meshes in the object
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

        // Clean up
        glDeleteProgram(shaderProgram);
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

    void onYawPitch(float dx, float dy) 
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

    void onKeys(const int& key) 
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

    std::vector<Mesh> getTargetObj() 
    {
        objl::Loader loader;
        if (loader.LoadFile(objFolder + "/" + targetFile) == 0) 
        {
            std::cout << "Failed to load object file" << std::endl;
        } 
        else 
        {
            std::cout << "Successfully loaded object file" << std::endl;
        }
        return objlMeshToCustomMesh(loader.LoadedMeshes);
    }
}