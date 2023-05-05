#include <iostream>

#include "renderer.h"

namespace renderer 
{
    std::string objFolder = "res"; // Must be in working directory
    std::vector<std::string> allObjNames = getObjFiles(objFolder);
    std::string targetFile = "teapot.obj"; // Don't forget .obj extension
    std::vector<Mesh> targetObj = getTargetObj();

    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
    glm::vec3 targetPos = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 lookDir = glm::vec3(0.0f, 0.0f, 1.0f);
    float cameraYaw = -M_PI_2; // -90 degrees
    float cameraPitch = 0.0f;
    Mesh cube(cubeVertices, cubeIndices);

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

    void drawObject(const std::vector<Mesh>& object, const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
        // Vertex shader source
        const GLchar *vertexShaderSource = R"glsl(
            #version 330 core
            layout (location = 0) in vec3 position;

            uniform mat4 model;
            uniform mat4 view;
            uniform mat4 projection;

            void main()
            {
                gl_Position = projection * view * model * vec4(position, 1.0);
            }
        )glsl";

        // Fragment shader source
        const GLchar *fragmentShaderSource = R"glsl(
            #version 330 core
            out vec4 color;
            void main()
            {
                color = vec4(1.0, 0.5, 0.2, 1.0);
            }
        )glsl";

        GLuint shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);

        // Set up vertex array and buffer objects
        GLuint VAO, VBO, EBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        // Loop through the meshes in the object
        for (const Mesh& mesh : object) {
            glBindVertexArray(VAO);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), mesh.vertices.data(), GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), mesh.indices.data(), GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
            glEnableVertexAttribArray(0);

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

    void updateCamera(float dx, float dy) {
        // Update camera yaw and pitch
        cameraYaw += dx;
        cameraPitch += dy;

        // Clamp pitch between -89 and 89 degrees
        if (cameraPitch > glm::radians(89.0f)) {
            cameraPitch = glm::radians(89.0f);
        }
        if (cameraPitch < glm::radians(-89.0f)) {
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

    std::vector<std::string> getObjFiles(const std::string& folderName) {
        std::vector<std::string> objFiles;

        try {
            for (const auto& entry : std::filesystem::directory_iterator(folderName)) {
                if (entry.is_regular_file() && entry.path().extension() == ".obj") {
                    objFiles.push_back(entry.path().string());
                }
            }
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }

        return objFiles;
    }

    std::vector<Mesh> getTargetObj() {
        objl::Loader loader;
        if (loader.LoadFile(objFolder + "/" + targetFile) == 0) {
            std::cout << "Failed to load object file" << std::endl;
        } else {
            std::cout << "Successfully loaded object file" << std::endl;
        }
        return objlMeshToCustomMesh(loader.LoadedMeshes);
    }

    std::vector<Mesh> objlMeshToCustomMesh(const std::vector<objl::Mesh>& objlMeshes) {
        std::vector<Mesh> customMeshes;

        for (const objl::Mesh& objlMesh : objlMeshes) {
            std::vector<Vertex> customVertices;
            std::vector<unsigned int> customIndices;

            for (const objl::Vertex& objlVertex : objlMesh.Vertices) {
                glm::vec3 position(objlVertex.Position.X, objlVertex.Position.Y, objlVertex.Position.Z);
                glm::vec3 normal(objlVertex.Normal.X, objlVertex.Normal.Y, objlVertex.Normal.Z);
                glm::vec2 textureCoordinate(objlVertex.TextureCoordinate.X, objlVertex.TextureCoordinate.Y);
                customVertices.emplace_back(position, normal, textureCoordinate);
            }

            customIndices = objlMesh.Indices;

            Mesh customMesh(customVertices, customIndices);
            customMesh.meshName = objlMesh.MeshName;

            const objl::Material& objlMat = objlMesh.MeshMaterial;
            Material customMaterial;
            customMaterial.name = objlMat.name;
            customMaterial.ka = glm::vec3(objlMat.Ka.X, objlMat.Ka.Y, objlMat.Ka.Z);
            customMaterial.kd = glm::vec3(objlMat.Kd.X, objlMat.Kd.Y, objlMat.Kd.Z);
            customMaterial.ks = glm::vec3(objlMat.Ks.X, objlMat.Ks.Y, objlMat.Ks.Z);
            customMaterial.ns = objlMat.Ns;
            customMaterial.ni = objlMat.Ni;
            customMaterial.d = objlMat.d;
            customMaterial.illum = objlMat.illum;
            customMaterial.kaMap = objlMat.map_Ka;
            customMaterial.kdMap = objlMat.map_Kd;
            customMaterial.ksMap = objlMat.map_Ks;
            customMaterial.nsMap = objlMat.map_Ns;
            customMaterial.dMap = objlMat.map_d;
            customMaterial.bumpMap = objlMat.map_bump;

            customMesh.meshMaterial = customMaterial;

            customMeshes.push_back(customMesh);
        }

        return customMeshes;
    }
}