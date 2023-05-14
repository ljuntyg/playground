#include <algorithm>

#include "ui.h"
#include "renderer.h"
#include "text.h"

namespace ui
{
    UIManager::UIManager(std::shared_ptr<UIRenderer> uiRenderer) : uiRenderer(uiRenderer) {}
    
    UIManager::~UIManager() {}

    void UIManager::addElement(std::shared_ptr<UIElement> element)
    {
        elements.push_back(element);
    }

    bool UIManager::handleInput(const SDL_Event& event)
    {
        for (const auto& element : elements)
        {
            if (recursiveHandleInput(event, element))
            {
                return true;
            }
        }

        return false; // Event was not handled by any element of the uiManager
    }


    bool UIManager::recursiveHandleInput(const SDL_Event& event, std::shared_ptr<UIElement> element)
    {
        if (element->handleInput(event))
        {
            return true;
        }

        // Check if any of the children handled the input
        for (const auto& child : element->children)
        {
            if (recursiveHandleInput(event, child))  // Recursively check input for children
            {
                return true;
            }
        }

        return false; // No child handled the input
    }

    void UIManager::render()
    {
        for (const auto& element : elements)
        {
            recursiveRender(element);
        }
    }

    void UIManager::recursiveRender(std::shared_ptr<UIElement> element)
    {
        element->render(*uiRenderer);

        // Render children after parent so they are rendered on top of parent
        for (const auto& child : element->children)
        {
            recursiveRender(child); // Recursively render children
        }
    }

    UIRenderer::UIRenderer(std::shared_ptr<renderer::Renderer> RENDERER) : RENDERER(RENDERER)
    {
        uiShaderProgram = renderer::createShaderProgram(UIVertexShaderSource, UIFragmentShaderSource);
        textShaderProgram = renderer::createShaderProgram(textVertexShaderSource, textFragmentShaderSource);
    }

    UIRenderer::~UIRenderer()
    {
        glDeleteProgram(uiShaderProgram);
        glDeleteProgram(textShaderProgram);
    }

    void UIRenderer::render(UIElement& element)
    {
        glUseProgram(uiShaderProgram);

        // Set up the model, view, and projection matrices
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(element.x, element.y, 0.0f)); // Translate ui element
        model = glm::scale(model, glm::vec3(element.width, element.height, 1.0f)); // Scale ui element
        glm::mat4 projection = glm::ortho(0.0f, this->RENDERER->WINDOW_WIDTH, 0.0f, this->RENDERER->WINDOW_HEIGHT);

        // Pass the matrices and color to the shader
        glUniformMatrix4fv(glGetUniformLocation(uiShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(uiShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform4fv(glGetUniformLocation(uiShaderProgram, "color"), 1, glm::value_ptr(element.color));

        // Bind the VAO
        glBindVertexArray(element.VAO);

        // Draw the square
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Unbind the VAO
        glBindVertexArray(0);
    }

    void UIRenderer::render(UIText& textElement)
    {
        glUseProgram(textShaderProgram);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(textElement.x, textElement.y, 0.0f)); // Translate ui element
        model = glm::scale(model, glm::vec3(textElement.width, textElement.height, 1.0f)); // Scale ui element
        glm::mat4 projection = glm::ortho(0.0f, this->RENDERER->WINDOW_WIDTH, 0.0f, this->RENDERER->WINDOW_HEIGHT);

        // Set the projection matrix in the shader
        glUniformMatrix4fv(glGetUniformLocation(textShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(textShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform4fv(glGetUniformLocation(textShaderProgram, "textColor"), 1, glm::value_ptr(textElement.color));

        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(textElement.VAO);

        // Iterate through all characters
        for (auto& ch : textElement.text->characters)
        {
            // Set the character's texture
            glBindTexture(GL_TEXTURE_2D, ch.font.textureIDs[ch.page]); // Find corresponding texture (in case there is more than 1 texture for the font)

            // Update VBO for each character
            GLfloat vertices[6][4];
            for (int i = 0; i < 6; ++i)
            {
                vertices[i][0] = ch.vertices[i*2];
                vertices[i][1] = ch.vertices[i*2+1];
                vertices[i][2] = ch.texCoords[i*2];
                vertices[i][3] = ch.texCoords[i*2+1];
            }

            // Render glyph texture over quad
            glBindBuffer(GL_ARRAY_BUFFER, textElement.VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

            // Render quad
            glDrawArrays(GL_TRIANGLES, 0, 6);
            
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    UIElement::UIElement(int x, int y, int width, int height, const glm::vec4& color, std::shared_ptr<ui::UIManager> uiManager)
        : x(x), y(y), width(width), height(height), color(color), uiManager(uiManager)
    {
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

        // Unbind the VAO
        glBindVertexArray(0);
    }

    UIElement::~UIElement() 
    {
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        glDeleteVertexArrays(1, &VAO);
    }

    void UIElement::addChild(std::shared_ptr<UIElement> element)
    {
        children.emplace_back(element);
        element->parent = shared_from_this();
        element->x += x; // Offset child x in relation to parent (this)
        element->y += y; // Offset child y in relation to parent (this)
    }

    UIBox::~UIBox() {}

    bool UIBox::handleInput(const SDL_Event& event) { return false; }

    void UIBox::render(UIRenderer& uiRenderer)
    {
        uiRenderer.render(*this);
    }

    UIButton::~UIButton() {}

    bool UIButton::handleInput(const SDL_Event& event)
    {
        if (event.type == SDL_MOUSEBUTTONDOWN)
        {
            // Get the mouse click coordinates
            int mouseX = event.button.x;
            int mouseY = event.button.y;

            // SDL coordinates are centered at top left with Y pointing down, OpenGL is centered at bottom left with Y up, adjust Y
            mouseY = this->uiManager->uiRenderer->RENDERER->WINDOW_HEIGHT - mouseY;

            // Check if the click is within the range of the UI window
            if (mouseX >= x && mouseX <= x + width && mouseY >= y && mouseY <= y + height)
            {
                // Handle the mouse click event here
                handleClick(mouseX, mouseY);
                return true; // Only return input handled if click handled (click was within element area)
            }
        }

        return false;
    }

    void UIButton::handleClick(int mouseX, int mouseY)
    {
        this->uiManager->uiRenderer->RENDERER->nextTargetObj();
    }

    void UIButton::render(UIRenderer& uiRenderer)
    {
        uiRenderer.render(*this);
    }

    UIText::UIText(std::shared_ptr<text::Text> text, int x, int y, int width, int height, const glm::vec4& color, std::shared_ptr<UIManager> uiManager) 
        : text(text), UIElement(x, y, width, height, color, uiManager) 
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    UIText::~UIText() 
    {
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }

    bool UIText::handleInput(const SDL_Event& event)
    {
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
        {
            text->textManager->nextFont();
            for (auto& t : text->textManager->texts)
            {
                t->calculateVertices();
            }
            return true;
        }

        return false;
    }

    void UIText::render(UIRenderer& uiRenderer)
    {
        uiRenderer.render(*this);
    }
}