#include <iostream>

#include "gui.h"
#include "shaders.h"

namespace gui
{
    GUIHandler::GUIHandler() {}

    GUIHandler::~GUIHandler() // GUIElement lifetime bound to GUIHandler
    {
        if (!elements.empty())
        {
            for (auto e : elements)
            {
                delete e;
            }
        }
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
        delete element;
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

    bool GUIHandler::handleGUIElementInput(GUIElement* element, const SDL_Event* event)
    {
        if (elements.find(element) == elements.end())
        {
            std::cerr << "Element to handle input for not present in GUIHandler, unable to handle input for element" << std::endl;
            return false;
        }

        return element->handleInput(event);
    }

    void GUIHandler::addToRenderQueue(GUIElement* element)
    {
        if (!element->isVisible)
        {
            std::cerr << "Attempt to add non-visible GUIElement to GUIHandler render queue, element not added to queue" << std::endl;
            return;
        }

        renderQueue.push(element);
    }

    void GUIHandler::addToHandleInputQueue(GUIElement* element)
    {
        if (!element->takesInput)
        {
            std::cerr << "Attempt to add GUIElement not accepting input to GUIHandler handle input queue, element not added to queue" << std::endl;
            return;
        }
        handleInputQueue.push(element);
    }

    bool GUIHandler::renderWholeQueue()
    {
        bool result = true;
        while (!renderQueue.empty())
        {
            if (!renderGUIElement(renderQueue.front()))
            {
                std::cerr << "Issue rendering individual GUIElement when rendering whole queue, popping GUIElement from queue anyway" << std::endl;
                result = false;
            }

            renderQueue.pop();
        }

        return result;
    }

    bool GUIHandler::handleInputWholeQueue(const SDL_Event* event)
    {
        bool result = true;
        while (!handleInputQueue.empty())
        {
            if(!handleGUIElementInput(handleInputQueue.front(), event))
            {
                std::cerr << "Issue handling input for individual GUIElement when handling input for whole queue, popping GUIElement from queue anyway" << std::endl;
                result = false;
            }

            handleInputQueue.pop();
        }

        return result;
    }

    GUIElement::GUIElement(GUIHandler* handler, int xPos, int yPos, int width, int height, bool isMovable, bool isVisible, bool takesInput)
        : handler(handler), xPos(xPos), yPos(yPos), width(width), height(height), isMovable(isMovable), isVisible(isVisible), takesInput(takesInput)
    {
        if (isMovable && !takesInput)
        {
            std::cerr << "Attempt to create movable GUIElement that does not take input, element made immovable" << std::endl;
            isMovable = false;
        }

        if (!initializeShaders())
        {
            std::cerr << "Shaders for GUIElement not initialized correctly" << std::endl;
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

        for (auto child : children)
        {
            delete child;
        }
    }

    const char* GUIElement::getGuiVertexShader()
    {
        return shaders::guiVertexShaderSource;
    }

    const char* GUIElement::getGuiFragmentShader()
    {
        return shaders::guiFragmentShaderSource;
    }

    bool GUIElement::initializeShaders()
    {
        shaderProgram = shaders::createShaderProgram(getGuiVertexShader(), getGuiFragmentShader());
        if (shaderProgram == 0)
        {
            return false;
        }

        // Get uniform locations
        modelLoc = glGetUniformLocation(shaderProgram, "model");
        viewLoc = glGetUniformLocation(shaderProgram, "view");
        projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        useTextureLoc = glGetUniformLocation(shaderProgram, "useTexture");
        objectColorLoc = glGetUniformLocation(shaderProgram, "objectColor");

        return true;
    }

    void GUIElement::addChild(GUIElement* child)
    {
        children.emplace_back(child);
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
        // Initialize vertices/buffers in separate function? Depends on if movable or not
    }

    bool GUIElement::handleInput(const SDL_Event* event)
    {
        // Basic GUIElement should handle input if and only if movable
    }
}