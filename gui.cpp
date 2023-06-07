#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <random>

#include "gui.h"
#include "shaders.h"

namespace gui
{
    GUIHandler::GUIHandler(float windowWidth, float windowHeight)
        : windowWidth(windowWidth), windowHeight(windowHeight) {}

    // GUIElement lifetime bound to GUIHandler
    // Seems like GUIElement members will be deleted (and removed from elements) before GUIHandler
    // Generally elements WILL be empty
    // "delete element" can be called to remove delete element and remove it from elements externally
    GUIHandler::~GUIHandler()
    {
        if (!elements.empty())
        {
            for (auto e : elements)
            {
                if (e != nullptr)
                {
                    delete e;
                }
            }
        }
    }

    float GUIHandler::getWindowWidth() const
    {
        return windowWidth;
    }

    float GUIHandler::getWindowHeight() const
    {
        return windowHeight;
    }

    void GUIHandler::addElement(GUIElement* element)
    {
        elements.emplace(element);
    }

    void GUIHandler::removeElement(GUIElement* element)
    {
        auto it = std::find(elements.begin(), elements.end(), element);
        if(it == elements.end())
        {
            std::cerr << "Element not a member of elements, unable to remove from elements" << std::endl;
            return;
        }

        elements.erase(it);
    }

    bool GUIHandler::renderGUIElement(GUIElement* element) const 
    {
        if (elements.find(element) == elements.end())
        {
            std::cerr << "Element to render not present in GUIHandler, unable to render element" << std::endl;
            return false;
        }

        return element->render();
    }

    bool GUIHandler::handleGUIElementInput(GUIElement* element, const SDL_Event* event, MouseState* mouseState)
    {
        if (elements.find(element) == elements.end())
        {
            std::cerr << "Element to handle input for not present in GUIHandler, unable to handle input for element" << std::endl;
            return false;
        }

        return element->handleInput(event, mouseState);
    }

    void GUIHandler::addToRenderVector(GUIElement* element)
    {
        if (!element->getIsVisible())
        {
            std::cerr << "Attempt to add non-visible GUIElement to GUIHandler render vector, element not added to vector" << std::endl;
            return;
        }

        renderVector.emplace_back(element);
    }

    void GUIHandler::addToHandleInputVector(GUIElement* element)
    {
        if (!element->getTakesInput())
        {
            std::cerr << "Attempt to add GUIElement not accepting input to GUIHandler handle input vector, element not added to vector" << std::endl;
            return;
        }
        
        handleInputVector.emplace_back(element);
    }

    bool GUIHandler::renderWholeVector()
    {
        bool result = true;
        for (auto e : renderVector)
        {
            if (!renderGUIElement(e))
            {
                std::cerr << "Issue rendering individual GUIElement when rendering all elements in renderVector" << std::endl;
                result = false;
            }
        }

        return result;
    }

    bool GUIHandler::handleInputWholeVector(const SDL_Event* event, MouseState* mouseState)
    {
        bool result = true;
        for (auto e : handleInputVector)
        {
            if(!handleGUIElementInput(e, event, mouseState))
            {
                std::cerr << "Issue handling input for individual GUIElement when handling input for all elements in handleInputVector" << std::endl;
                result = false;
            }
        }

        return result;
    }

    GUIElement::GUIElement(GUIHandler* handler, int xPos, int yPos, int width, int height, glm::vec4 color, bool isMovable, bool isVisible, bool takesInput)
        : handler(handler), xPos(xPos), yPos(yPos), width(width), height(height), color(color), isMovable(isMovable), isVisible(isVisible), takesInput(takesInput)
    {
        // TODO: Ensure xPos, yPos, width and height places element within screen (width and height clip outside screen?), also needs to be checked on handleInput/render

        if (isMovable && !takesInput)
        {
            std::cerr << "Attempt to create movable GUIElement that does not take input, element made immovable" << std::endl;
            isMovable = false;
        }

        if (isMovable || takesInput)
        {
            handler->addToHandleInputVector(this);
        }

        if (isVisible)
        {
            handler->addToRenderVector(this);
        }

        if (!initializeShaders())
        {
            std::cerr << "Shaders for GUIElement not initialized correctly" << std::endl;
        }

        if (!initializeBuffers())
        {
            std::cerr << "Buffers for GUIElement failed to initialize" << std::endl;
        }

        handler->addElement(this);
    }

    GUIElement::~GUIElement()
    {
        // OpenGL clean-up
        glDeleteProgram(shaderProgram);
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);

        handler->removeElement(this);
    }

    bool GUIElement::getIsMovable()
    {
        return isMovable;
    }

    bool GUIElement::getIsVisible()
    {
        return isVisible;
    }

    bool GUIElement::getTakesInput()
    {
        return takesInput;
    }

    const char* GUIElement::getVertexShader()
    {
        return shaders::guiVertexShaderSource;
    }

    const char* GUIElement::getFragmentShader()
    {
        return shaders::guiFragmentShaderSource;
    }

    bool GUIElement::initializeShaders()
    {
        shaderProgram = shaders::createShaderProgram(getVertexShader(), getFragmentShader());
        if (shaderProgram == 0)
        {
            return false;
        }

        // Get uniform locations
        modelLoc = glGetUniformLocation(shaderProgram, "model");
        viewLoc = glGetUniformLocation(shaderProgram, "view");
        projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        useTextureLoc = glGetUniformLocation(shaderProgram, "useTexture");
        colorLoc = glGetUniformLocation(shaderProgram, "color");
        textLoc = glGetUniformLocation(shaderProgram, "text");

        return true;
    }

    bool GUIElement::initializeBuffers()
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

        // Check for OpenGL errors
        GLenum error = glGetError();
        if (error != GL_NO_ERROR)
        {
            std::cerr << "Error when initializing GUIElement buffers, OpenGL error: " << error << std::endl;
            return false;
        }
        
        return true;
    }

    void GUIElement::addChild(GUIElement* child)
    {
        if (child->handler != handler)
        {
            std::cerr << "Child handler is not the same as parent handler, unable to add child GUIElement" << std::endl;
            return;
        }

        children.emplace_back(child);
        child->xPos += xPos; // Offset child x in relation to parent (this)
        child->yPos += yPos; // Offset child y in relation to parent (this)
    }

    void GUIElement::removeChild(GUIElement* child)
    {
        auto it = std::find(children.begin(), children.end(), child);
        if(it == children.end())
        {
            std::cerr << "Child not a member of children, unable to remove from children" << std::endl;
            return;
        }

        children.erase(it);
        delete child;
    }

    bool GUIElement::render() const
    {
        glUseProgram(shaderProgram);

        // Set up the model, view, and projection matrices
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(xPos, yPos, 0.0f)); // Translate ui element
        model = glm::scale(model, glm::vec3(width, height, 1.0f)); // Scale ui element
        glm::mat4 projection = glm::ortho(0.0f, handler->getWindowWidth(), 0.0f, handler->getWindowHeight());

        // Pass the matrices and color to the shader
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform4fv(colorLoc, 1, glm::value_ptr(color));

        // Bind the VAO
        glBindVertexArray(VAO);

        // Draw the square
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Unbind the VAO
        glBindVertexArray(0);

        return true;
    }

    bool GUIElement::handleInput(const SDL_Event* event, MouseState* mouseState)
    {
        // Only GUIElement that isMovable should be movable
        if (isMovable)
        {
            move(event, mouseState);
        }

        return true;
    }

    void GUIElement::move(const SDL_Event* event, MouseState* mouseState)
    {
        switch (event->type)
        {
        case SDL_MOUSEBUTTONDOWN:
            if (event->button.button == SDL_BUTTON_LEFT)
            {
                int mouseX = event->button.x;
                int mouseY = (int)(handler->getWindowHeight() - event->button.y); // Conversion from top-left to bottom-left system
                if (mouseX >= xPos && mouseX <= xPos + width &&
                    mouseY >= yPos && mouseY <= yPos + height)
                {
                    isBeingDragged = true;
                    *mouseState = GUIControl;
                }
            }
            break;

        case SDL_MOUSEBUTTONUP:
            if (event->button.button == SDL_BUTTON_LEFT)
            {
                isBeingDragged = false;
                *mouseState = CameraControl;
            }
            break;

        case SDL_MOUSEMOTION:
            if (isBeingDragged)
            {
                int xOffset = event->motion.xrel;
                int yOffset = event->motion.yrel;

                xPos += xOffset;
                yPos -= yOffset; // Conversion from top-left to bottom-left system

                offsetChildren(xOffset, yOffset);
            }
            break;
        }
    }

    void GUIElement::offsetChildren(int xOffset, int yOffset)
    {
        for (auto child : children)
        {
            child->xPos += xOffset;
            child->yPos -= yOffset; // Conversion from top-left to bottom-left system

            child->offsetChildren(xOffset, yOffset);
        }
    }

    GUIButton::GUIButton(GUIHandler* handler, int xPos, int yPos, int width, int height, glm::vec4 color,
        std::function<void(GUIButton*)> onClick, bool isMovable, bool isVisible, bool takesInput)
        : GUIElement(handler, xPos, yPos, width, height, color, isMovable, isVisible, takesInput), onClick(onClick) {}

    GUIButton::~GUIButton() {}

    bool GUIButton::handleInput(const SDL_Event* event, MouseState* mouseState) 
    {
        if (isMovable)
        {
            move(event, mouseState);
        }

        if (event->type == SDL_MOUSEBUTTONUP) 
        {
            if (event->button.button == SDL_BUTTON_LEFT) 
            {
                int mouseX = event->button.x;
                int mouseY = (int)(handler->getWindowHeight() - event->button.y); // Conversion from top-left to bottom-left system
                if (mouseX >= xPos && mouseX < xPos + width && 
                    mouseY >= yPos && mouseY < yPos + height) 
                {
                    onClick(this);
                    return true;
                }
            }

            return false;
        }

        return true;
    }

    void GUIButton::randomColor(GUIButton* button)
    {
        // Mersenne Twister pseudo-random number generator
        static std::mt19937 rng(std::random_device{}());

        // Uniform distribution between 0.0 and 1.0
        static std::uniform_real_distribution<float> distribution(0.0f, 1.0f);

        float r = distribution(rng);
        float g = distribution(rng);
        float b = distribution(rng);

        button->color = glm::vec4(r, g, b, 1.0f);
    }

    void GUIButton::quitApplication(GUIButton* button)
    {
        std::cerr << "Quit application button method not implemented" << std::endl;
    }

    GUIText::GUIText(GUIHandler* handler, int xPos, int yPos, int width, int height, glm::vec4 color,
        std::vector<text::Character*>* text, bool isMovable, bool isVisible, bool takesInput)
        : GUIElement(handler, xPos, yPos, width, height, color, isMovable, isVisible, takesInput), text(text)
    {
        if (!generateVertices())
        {
            std::cerr << "Failed to render vertices for text provided to GUIText";
        }

        if (!loadFontTextures())
        {
            std::cerr << "Failed to load textures for a font belonging to text provided to GUIText" << std::endl;
        }
    }

    GUIText::~GUIText() 
    {
        // TODO: Do something with text pointer
        for (auto &pair : fontTextures)
        {
            for (GLuint textureID : pair.second)
            {
                glDeleteTextures(1, &textureID);
            }
        }
    }

    const char* GUIText::getVertexShader()
    {
        return shaders::textVertexShaderSource;
    }

    const char* GUIText::getFragmentShader()
    {
        return shaders::textFragmentShaderSource;
    }

    bool GUIText::render() const
    {
        return true;
    }

    bool GUIText::handleInput(const SDL_Event* event, MouseState* mouseState)
    {
        return true;
    }

    bool GUIText::generateVertices()
    {
        return true;
    }

    bool GUIText::loadFontTextures()
    {
        std::vector<text::Font*> fonts;
        for (text::Character* ch : *text)
        {
            if (std::find(fonts.begin(), fonts.end(), ch->font) == fonts.end())
            {
                fonts.emplace_back(ch->font);
            }
        }

        for (text::Font* font : fonts) 
        {
            std::vector<GLuint> textureIDs;
            for (auto& texture : font->getTextures()) 
            {
                if (!texture)
                {
                    std::cerr << "Invalid texture in loadFontTexture" << std::endl;
                }

                // Load the texture into OpenGL and store the texture ID
                GLuint textureID;
                glGenTextures(1, &textureID);
                glBindTexture(GL_TEXTURE_2D, textureID);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                if(font->getTextureNbrChannels() == 3)
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)font->getTextureWidth(), (GLsizei)font->getTextureHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, texture);
                else if(font->getTextureNbrChannels() == 4)
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)font->getTextureWidth(), (GLsizei)font->getTextureHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, texture);
                glGenerateMipmap(GL_TEXTURE_2D);

                textureIDs.push_back(textureID);
            }

            fontTextures[font] = textureIDs;
        }

        return true;
    }

    GUIElement* GUIElementFactory::createGUIElement(GUIHandler* handler, int xPos, int yPos, int width, int height, glm::vec4 color, bool isMovable, bool isVisible, bool takesInput) {
        GUIElement* element = new GUIElement(handler, xPos, yPos, width, height, color, isMovable, isVisible, takesInput);
        if (element->initializeShaders() && element->initializeBuffers()) {
            return element;
        } else {
            delete element;
            return nullptr;
        }
    }

    GUIButton* GUIElementFactory::createGUIButton(GUIHandler* handler, int xPos, int yPos, int width, int height, glm::vec4 color, std::function<void(GUIButton*)> onClick, bool isMovable, bool isVisible, bool takesInput) {
        GUIButton* buttonElement = new GUIButton(handler, xPos, yPos, width, height, color, onClick, isMovable, isVisible, takesInput);
        if (buttonElement->initializeShaders() && buttonElement->initializeBuffers()) {
            return buttonElement;
        } else {
            delete buttonElement;
            return nullptr;
        }
    }

    GUIText* GUIElementFactory::createGUIText(GUIHandler* handler, int xPos, int yPos, int width, int height, glm::vec4 color, std::vector<text::Character*>* text, bool isMovable, bool isVisible, bool takesInput) {
        GUIText* textElement = new GUIText(handler, xPos, yPos, width, height, color, text, isMovable, isVisible, takesInput);
        if (textElement->initializeShaders() && textElement->initializeBuffers() && textElement->generateVertices() && textElement->loadFontTextures()) {
            return textElement;
        } else {
            delete textElement;
            return nullptr;
        }
    }
}