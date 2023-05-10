#include "ui.h"
#include "renderer.h"

namespace ui
{
    UIManager::UIManager(std::shared_ptr<UIRenderer> renderer) : renderer(renderer) {}
    
    UIManager::~UIManager() {}

    void UIManager::addElement(std::shared_ptr<UIElement> element)
    {
        elements.push_back(element);
    }

    void UIManager::handleInput(const SDL_Event& event)
    {
        for (const auto& element : elements)
        {
            element->handleInput(event);
        }
    }

    void UIManager::render()
    {
        for (const auto& element : elements)
        {
            element->render(*renderer);
        }
    }

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

            uniform vec4 color;

            void main()
            {
                FragColor = color;
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

        // Pass the matrices and color to the shader
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform4fv(glGetUniformLocation(shaderProgram, "color"), 1, glm::value_ptr(element.color));

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

        // Render children here at bottom so they are rendered on top of parent
        if (!element.children.empty())
        {
            for (const auto& child : element.children)
            {
                render(*child);
            }
        }
    }

    UIElement::UIElement(int x, int y, int width, int height, const glm::vec4& color)
        : x(x), y(y), width(width), height(height), color(color) {}

    UIElement::~UIElement() {}

    void UIElement::addChild(std::shared_ptr<UIElement> element)
    {
        children.emplace_back(element);
        element->parent = shared_from_this();
        element->x += x; // Offset child x in relation to parent (this)
        element->y += y; // Offset child y in relation to parent (this)
    }

    UIBox::~UIBox() {}

    void UIBox::handleInput(const SDL_Event& event) {}

    void UIBox::render(UIRenderer& renderer)
    {
        renderer.render(*this);
    }

    UIButton::~UIButton() {}

    void UIButton::handleInput(const SDL_Event& event)
    {
        if (event.type == SDL_MOUSEBUTTONDOWN)
        {
            // Get the mouse click coordinates
            int mouseX = event.button.x;
            int mouseY = event.button.y;

            // SDL coordinates are centered at top left with Y pointing down, OpenGL is centered at bottom left with Y up, adjust Y
            mouseY = renderer::WINDOW_HEIGHT - mouseY;

            // Check if the click is within the range of the UI window
            if (mouseX >= x && mouseX <= x + width && mouseY >= y && mouseY <= y + height)
            {
                // Handle the mouse click event here
                handleClick(mouseX, mouseY);
            }
        }
    }

    void UIButton::handleClick(int mouseX, int mouseY)
    {
        renderer::nextTargetObj();
    }

    void UIButton::render(UIRenderer& renderer)
    {
        renderer.render(*this);
    }
}