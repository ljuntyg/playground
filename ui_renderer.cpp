#include "ui.h"
#include "renderer.h"

namespace ui
{
    UIRenderer::UIRenderer()
    {
        vertexShaderSource = R"(
            #version 330 core
            layout (location = 0) in vec2 aPos;
            layout (location = 1) in vec2 aTexCoord;

            out vec2 TexCoord;

            uniform mat4 projection;
            uniform mat4 model;

            void main()
            {
                gl_Position = projection * model * vec4(aPos, 0.0, 1.0);
                TexCoord = aTexCoord;
            })";

        fragmentShaderSource = R"(
            #version 330 core
            out vec4 FragColor;

            in vec2 TexCoord;

            uniform vec4 color;

            void main()
            {
                FragColor = color;
            })";

        // Initialize the shader program
        shaderProgram = renderer::createShaderProgram(vertexShaderSource, fragmentShaderSource);

        // Set up the orthogonal projection matrix, model set in render
        projection = glm::ortho(0.0f, renderer::WINDOW_WIDTH, 0.0f, renderer::WINDOW_HEIGHT);

        // Generate the VAO, VBO, EBO
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
    }

    UIRenderer::~UIRenderer()
    {
        // Clean up OpenGL resources
        glDeleteProgram(shaderProgram);
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

    void UIRenderer::render(UIElement& element)
    {
        model = glm::translate(glm::mat4(1.0f), glm::vec3(element.x, element.y, 0.0f));

        std::vector<GLfloat> vertices = {
            0.0f, 0.0f, 0.0f, 0.0f,
            (float)element.width, 0.0f, 1.0f, 0.0f,
            (float)element.width, (float)element.height, 1.0f, 1.0f,
            0.0f, (float)element.height, 0.0f, 1.0f
        };

        std::vector<GLuint> indices = {
            0, 1, 2,
            2, 3, 0
        };

        // Bind the VAO
        glBindVertexArray(VAO);

        // Update the VBO data
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

        // Update the EBO data
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

        // Enable and specify the vertex attributes
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid *)(2 * sizeof(GLfloat)));

        // Use the shader program
        glUseProgram(shaderProgram);

        // Set the projection uniform
        GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Set the model uniform
        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        // Set the color uniform (example: white)
        GLint colorLoc = glGetUniformLocation(shaderProgram, "color");
        glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f);

        // Draw the element
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        // Unbind the VAO
        glBindVertexArray(0);
    }
}