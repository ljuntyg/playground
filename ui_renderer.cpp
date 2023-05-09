#include "ui.h"
#include "renderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace ui
{
    UIRenderer::UIRenderer()
    {
        vertexShaderSource = R"(
            #version 330 core
            layout (location = 0) in vec3 aPos;

            uniform mat4 model;
            uniform mat4 view;
            uniform mat4 projection;

            void main()
            {
                gl_Position = projection * view * model * vec4(aPos, 1.0);
            }
        )";

        fragmentShaderSource = R"(
            #version 330 core
            out vec4 FragColor;

            void main()
            {
                FragColor = vec4(1.0, 0.5, 0.2, 1.0);
            }
        )";
    }

    UIRenderer::~UIRenderer()
    {
        // Clean up OpenGL resources
        glDeleteProgram(shaderProgram);
    }

    void UIRenderer::render(UIElement& element)
    {
        shaderProgram = renderer::createShaderProgram(vertexShaderSource, fragmentShaderSource);

        // Define 1x1 square with bottom-left square corner at origin
        float vertices[] = {
            0.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f
        };

        // Define the square's indices
        unsigned int indices[] = {
            0, 1, 2,
            2, 3, 0
        };

        GLuint VAO, VBO, EBO;

        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // Set the vertex attributes pointers
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Set up the model, view, and projection matrices
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(element.x, element.y, 0.0f)); // Translate ui element
        model = glm::scale(model, glm::vec3(element.width, element.height, 1.0f)); // Scale ui element
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::ortho(0.0f, renderer::WINDOW_WIDTH, 0.0f, renderer::WINDOW_HEIGHT);

        // Use the shader program
        glUseProgram(shaderProgram);

        // Pass the matrices to the shader
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // Draw the square
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Clean up
        glDisableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        glDeleteVertexArrays(1, &VAO);
        glDeleteProgram(shaderProgram);
    }
}