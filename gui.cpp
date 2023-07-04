#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <random>
#include <locale>
#include <codecvt>

#include "gui.h"
#include "shaders.h"
#include "texture_loader.h"

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
        if (!zIndexRootElementMap.empty())
        {
            for (auto& pair : zIndexRootElementMap)
            {
                auto e = pair.second;
                if (e != nullptr)
                {
                    delete e;
                }
            }
        }
    }

    void GUIHandler::notify(const event::Event* event) 
    {
        if (const event::WindowResizeEvent* resizeEvent = dynamic_cast<const event::WindowResizeEvent*>(event)) 
        {
            windowWidth = (float)resizeEvent->newWidth;
            windowHeight = (float)resizeEvent->newHeight;
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

    void GUIHandler::addElement(GUIElement* element, int zIndex)
    {
        zIndexRootElementMap.emplace(zIndex, element);
    }

    void GUIHandler::removeElement(GUIElement* element)
    {
        for (auto it = zIndexRootElementMap.begin(); it != zIndexRootElementMap.end(); ++it)
        {
            if(it->second == element)
            {
                zIndexRootElementMap.erase(it);
                return;  // Return after the first matching element is found and removed
            }
        }

        std::cerr << "Element not a member of elements, unable to remove from elements" << std::endl;
    }

    void GUIHandler::prepareGUIRendering() const
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);
    }

    void GUIHandler::finishGUIRendering() const
    {
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
    }

    bool GUIHandler::renderAllElements() const
    {
        prepareGUIRendering();

        bool result = true;
        for (auto& pair : zIndexRootElementMap)
        {
            auto e = pair.second;
            if (e->getIsVisible())
            {
                if (!e->render())
                {
                    std::cerr << "Issue rendering individual GUIElement when rendering all elements in renderVector" << std::endl;
                    result = false;
                }
            }
        }

        finishGUIRendering();

        return result;
    }

    bool GUIHandler::receiveInputAllElements(const SDL_Event* event, InputState* inputState)
    {
        int i = (int)zIndexRootElementMap.size();
        for (auto it = zIndexRootElementMap.rbegin(); it != zIndexRootElementMap.rend(); ++it)
        {
            auto e = it->second;
            /* std::cout << "ELEMENT " << i << " position, x: " << e->getXPos() << ", y: " << e->getYPos() << std::endl; */
            if (e->receiveInput(event, inputState))
            {
                int maxZIndex = zIndexRootElementMap.rbegin()->first;  // Get max zIndex after consuming the event
                if (maxZIndex > 1000)
                {
                    normalizeZIndices();
                    maxZIndex = zIndexRootElementMap.rbegin()->first;
                }

                /* std::cout << "ELEMENT " << i << " consumed event " << event->type << ", current maxZIndex: " << maxZIndex << std::endl; */
                removeElement(e); // Remove element from zIndexRootElementMap with now old zIndex
                addElement(e, maxZIndex + 1); // Re-add element to zIndexRootElementMap with new zIndex
                return true; // Stop propagation on event consumed
            }
            --i;
            /* std::cout << std::endl; */
        }

        return false; // No element consumed the event
    }

    std::multimap<int, GUIElement*> GUIHandler::getZIndexRootElementMap()
    {
        return zIndexRootElementMap;
    }

    void GUIHandler::normalizeZIndices()
    {
        std::multimap<int, GUIElement*> newZIndexRootElementMap;
        int newZIndex = 0;

        for (auto& pair : zIndexRootElementMap)
        {
            newZIndexRootElementMap.emplace(newZIndex++, pair.second);
        }

        zIndexRootElementMap.swap(newZIndexRootElementMap);
    }

    GUIElement* GUIHandler::getActiveElement()
    {
        return activeElement;
    }
    
    // TODO: Use dynamic cast to make changes when an activeElement
    // of certain type is swapped out, e.g. beingEdited for edit text
    void GUIHandler::setActiveElement(GUIElement* element)
    {
        activeElement = element;
    }

    bool GUIHandler::isOrContainsActiveElement(GUIElement* element)
    {   
        if (activeElement == element)
        {
            return true;
        }

        for (auto& child : element->getChildren())
        {
            if (isOrContainsActiveElement(child))
            {
                return true;
            }
        }

        return false;
    }

    GUIElement::GUIElement(GUIHandler* handler, int xPos, int yPos, int width, int height, bool isMovable, bool isResizable, bool isVisible, bool takesInput, int borderWidth, int cornerRadius, glm::vec4 color)
        : handler(handler), xPos(xPos), yPos(yPos), width(width), height(height), isMovable(isMovable), isResizable(isResizable), isVisible(isVisible), takesInput(takesInput), borderWidth(borderWidth), cornerRadius(cornerRadius), color(color)
    {
        if (handler == nullptr)
        {
            std::cerr << "Null handler passed to GUIText constructor, returning early" << std::endl;
            return;
        }

        int zIndex = 0;
        if (!handler->getZIndexRootElementMap().empty())
        {
            zIndex = std::prev(handler->getZIndexRootElementMap().end())->first + 1;
        }
        handler->addElement(this, zIndex);
    }

    GUIElement::~GUIElement()
    {
        // OpenGL clean-up
        glDeleteProgram(shaderProgram);
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);

        for (auto& child : children) 
        {
            delete child;
        }
    }

    void GUIElement::addChild(GUIElement* child)
    {
        if (child->handler != handler)
        {
            std::cerr << "Child handler is not the same as parent handler, unable to add child GUIElement" << std::endl;
            return;
        }

        // Remove child from parent handler rootElements since it is no longer a root
        handler->removeElement(child);

        children.emplace(child);
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

        // Pass the variables to the shader
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform4fv(colorLoc, 1, glm::value_ptr(color));
        glUniform1f(timeLoc, (GLfloat)SDL_GetTicks() / 1000.0f); // In seconds

        // TODO: These don't need to be passed on every render call
        glUniform2f(resolutionLoc, (GLfloat)width, (GLfloat)height);
        glUniform1f(cornerRadiusLoc, (GLfloat)cornerRadius);

        // Bind the VAO
        glBindVertexArray(VAO);

        // Draw the square
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Unbind the VAO
        glBindVertexArray(0);

        // Check for OpenGL errors
        GLenum error = glGetError();
        if (error != GL_NO_ERROR)
        {
            std::cerr << "Error when rendering GUIElement, OpenGL error: " << error << std::endl;
            return false;
        }

        // Render children
        renderChildren();

        return true;
    }

    bool GUIElement::renderChildren() const
    {
        for (auto& child : children)
        {
            // TODO: This will effectively make any children of this element invisible too if this element is invisible, is this desired?
            if (isVisible)
            {
                if (!child->render())
                {
                    std::cerr << "Issue rendering individual GUIElement when rendering children" << std::endl;
                    return false;
                }
            }
        }

        return true;
    }

    bool GUIElement::handleInput(const SDL_Event* event, InputState* inputState)
    {
        return false; // Default implementation does not consume event
    }

    bool GUIElement::receiveInput(const SDL_Event* event, InputState* inputState)
    {
        if (handler->getActiveElement() && !handler->isOrContainsActiveElement(this))
        {
            return false;
        }

        if (isResizable && resize(event, inputState))
        {
            onResize();
            return true;
        }

        if (isMovable && move(event, inputState))
        {
            return true;
        }

        if (takesInput && handleInput(event, inputState))
        {
            return true;
        }

        return receiveInputChildren(event, inputState); // Propagate to children
    }

    bool GUIElement::receiveInputChildren(const SDL_Event* event, InputState* inputState)
    {
        for (auto& child : children)
        {
            if (child->receiveInput(event, inputState))
            {
                return true; // Event consumed, stop propagation
            }
        }

        return false;
    }

    bool GUIElement::isOnElement(int x, int y)
    {
        if (x >= xPos && x <= (xPos + width) && y >= yPos && y <= (yPos + height))
        {
            return true;
        }

        return false;
    }

    // 1 = top left, 2 = top right, 3 = bottom left, 4 = bottom right
    bool GUIElement::isOnCorner(int cornerNbr, int x, int y)
    {
        int newX = x, newY = y;
        findPossibleNewCorner(cornerNbr, &newX, &newY);
        int centerX = 0, centerY = 0;
        switch (cornerNbr)
        {
        case 1:
            centerX = xPos;
            centerY = yPos + height;
            break;
        case 2:
            centerX = xPos + width;
            centerY = yPos + height;
            break;
        case 3:
            centerX = xPos;
            centerY = yPos;
            break;
        case 4:
            centerX = xPos + width;
            centerY = yPos;
            break;
        default:
            std::cerr << "Passed integer not in range 1-4 to isOnCorner, returning false" << std::endl;
            return false;
        }

        int startBoundX = centerX - borderWidth;
        int endBoundX = centerX + borderWidth;
        int startBoundY = centerY - borderWidth;
        int endBoundY = centerY + borderWidth;

        if (x >= startBoundX && x <= endBoundX && y >= startBoundY && y <= endBoundY)
        {
            return true;
        }

        return false;
    }

    void GUIElement::findPossibleNewCorner(int cornerNbr, int* newX, int* newY) {}

    // Sets cornerNbrBeingResized to the corner being resized, or 0 if none being resized
    bool GUIElement::isOnAnyCorner(int x, int y, int* cornerNbrBeingResized)
    {
        for (int i = 1; i <= 4; ++i)
        {
            *cornerNbrBeingResized = i;
            if (isOnCorner(i, x, y))
            {
                return true;
            }
        }

        *cornerNbrBeingResized = 0;
        return false;
    }

    std::unordered_set<GUIElement*> GUIElement::getChildren()
    {
        return children;
    }

    int GUIElement::getXPos()
    {
        return xPos;
    }

    int GUIElement::getYPos()
    {
        return yPos;
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

    ElementManipulationState GUIElement::getManipulationStateResize()
    {
        return manipulationStateResize;
    }

    ElementManipulationState GUIElement::getManipulationStateMove()
    {
        return manipulationStateMove;
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
        timeLoc = glGetUniformLocation(shaderProgram, "time");
        resolutionLoc = glGetUniformLocation(shaderProgram, "resolution");
        cornerRadiusLoc = glGetUniformLocation(shaderProgram, "cornerRadius");

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

    void GUIElement::onResize() {}

    bool GUIElement::resize(const SDL_Event* event, InputState* inputState)
    {
        switch (event->type)
        {
        case SDL_MOUSEBUTTONDOWN:
            if (event->button.button == SDL_BUTTON_LEFT)
            {
                int mouseX = event->button.x;
                int mouseY = (int)(handler->getWindowHeight() - event->button.y); // Conversion from top-left to bottom-left system
                if (isOnAnyCorner(mouseX, mouseY, &cornerNbrBeingResized))
                {
                    handler->setActiveElement(this);
                    manipulationStateResize = ElementManipulationState::PressOnCorner;
                    inputState->setMouseState(InputState::GUIResizeControl);
                    return true;
                }
            }
            break;

        case SDL_MOUSEBUTTONUP:
            if (event->button.button == SDL_BUTTON_LEFT)
            {
                handler->setActiveElement(nullptr);
                inputState->setMouseState(InputState::CameraControl);
                cornerNbrBeingResized = 0;

                // Two possibilites on MOUSEBUTTONUP, either resizing or not, if resizing
                // then return true to consume event, else allow other elements to handle event
                if (manipulationStateResize == ElementManipulationState::Resizing)
                {
                    manipulationStateResize = ElementManipulationState::None;
                    return true;
                }
                else
                {
                    manipulationStateResize = ElementManipulationState::None;
                    return false;
                }
            }
            break;

        case SDL_MOUSEMOTION:
            if (manipulationStateResize == ElementManipulationState::PressOnCorner
                || manipulationStateResize == ElementManipulationState::Resizing)
            {
                manipulationStateResize = ElementManipulationState::Resizing;

                int xOffset = event->motion.xrel;
                int yOffset = -event->motion.yrel;

                switch (cornerNbrBeingResized)
                {
                case 1: // Top Left
                    if(width - xOffset >= 2 * borderWidth && height + yOffset >= 2 * borderWidth)
                    {
                        xPos += xOffset;
                        width -= xOffset;
                        height += yOffset;
                        offsetChildren(xOffset, 0); // xPos/yPos
                        resizeChildren(-xOffset, yOffset); // Width/height
                        return true;
                    }
                    break;

                case 2: // Top Right
                    if(width + xOffset >= 2 * borderWidth && height + yOffset >= 2 * borderWidth)
                    {
                        width += xOffset;
                        height += yOffset;
                        // offsetChildren(0, 0);
                        resizeChildren(xOffset, yOffset);
                        return true;
                    }
                    break;

                case 3: // Bottom Left
                    if(width - xOffset >= 2 * borderWidth && height - yOffset >= 2 * borderWidth)
                    {
                        xPos += xOffset;
                        yPos += yOffset;
                        width -= xOffset;
                        height -= yOffset;
                        offsetChildren(xOffset, -yOffset);
                        resizeChildren(-xOffset, -yOffset);
                        return true;
                    }
                    break;

                case 4: // Bottom Right
                    if(width + xOffset >= 2 * borderWidth && height - yOffset >= 2 * borderWidth)
                    {
                        yPos += yOffset;
                        width += xOffset;
                        height -= yOffset;
                        offsetChildren(0, -yOffset);
                        resizeChildren(xOffset, -yOffset);
                        return true;
                    }
                    break;
                
                // Default case (cornerNbrBeingResized not in range 1-4) means no corner is being resized
                default:
                    return false;
                }
            }
            break;

        default:
            break;
        }

        return false;
    }

    // Resizes children down to a minSize, any offset past minSize is stored in element accumUnderMinSizeX and accumUnderMinSizeY,
    // when element is upsized again, it checks against accumulations to ensure children are only upsized if they have "made up" for their
    // size underflow, this ensures they are only upsized again when their parent is the same size it was at the point when the minSize was reached
    void GUIElement::resizeChildren(int xOffset, int yOffset)
    {
        int minSize = 0;

        for (auto& child : children)
        {
            int newWidth = child->width + xOffset;
            int newHeight = child->height + yOffset;

            // Handle newWidth
            if (newWidth < minSize)
            {
                accumUnderMinSizeX += newWidth - minSize;
                newWidth = minSize;
            }
            else if (accumUnderMinSizeX < 0) // If there is accumulated size from previous underflow
            {
                int potentialWidth = newWidth + accumUnderMinSizeX;
                if (potentialWidth >= minSize) // New size is valid
                {
                    newWidth = potentialWidth;
                    accumUnderMinSizeX = 0; // Reset accumulation
                }
                else // Update accumulation
                {
                    accumUnderMinSizeX = potentialWidth - minSize;
                    newWidth = minSize;
                }
            }

            // Handle newHeight
            if (newHeight < minSize)
            {
                accumUnderMinSizeY += newHeight - minSize;
                newHeight = minSize;
            }
            else if (accumUnderMinSizeY < 0) // If there is accumulated size from previous underflow
            {
                int potentialHeight = newHeight + accumUnderMinSizeY;
                if (potentialHeight >= minSize) // New size is valid
                {
                    newHeight = potentialHeight;
                    accumUnderMinSizeY = 0; // Reset accumulation
                }
                else // Update accumulation
                {
                    accumUnderMinSizeY = potentialHeight - minSize;
                    newHeight = minSize;
                }
            }

            child->width = newWidth;
            child->height = newHeight;

            child->resizeChildren(xOffset, yOffset);

            // Override to handle resize changes (e.g. for GUIText text updating on resize)
            child->onResize();
        }
    }

    bool GUIElement::move(const SDL_Event* event, InputState* inputState)
    {
        switch (event->type)
        {
        case SDL_MOUSEBUTTONDOWN:
            if (event->button.button == SDL_BUTTON_LEFT)
            {
                int mouseX = event->button.x;
                int mouseY = (int)handler->getWindowHeight() - event->button.y; // Conversion from top-left to bottom-left system
                if (isOnElement(mouseX, mouseY))
                {
                    handler->setActiveElement(this);
                    manipulationStateMove = ElementManipulationState::PressOnElement;
                    inputState->setMouseState(InputState::GUIMouseControl);
                    return true;
                }
            }
            break;

        case SDL_MOUSEBUTTONUP:
            if (event->button.button == SDL_BUTTON_LEFT)
            {
                handler->setActiveElement(nullptr);
                inputState->setMouseState(InputState::CameraControl);

                // Two possibilites on MOUSEBUTTONUP, either resizing or not, if resizing
                // then return true to consume event, else allow other elements to handle event
                if (manipulationStateMove == ElementManipulationState::Dragging)
                {
                    manipulationStateMove = ElementManipulationState::None;
                    return true;
                }
                else
                {
                    manipulationStateMove = ElementManipulationState::None;
                    return false;
                }
            }
            break;

        case SDL_MOUSEMOTION:
            if (manipulationStateMove == ElementManipulationState::PressOnElement
                || manipulationStateMove == ElementManipulationState::Dragging)
            {
                manipulationStateMove = ElementManipulationState::Dragging;

                int xOffset = event->motion.xrel;
                int yOffset = event->motion.yrel;

                xPos += xOffset;
                yPos -= yOffset;

                offsetChildren(xOffset, yOffset);
                return true;
            }
            break;

        default:
            break;
        }

        return false;
    }

    void GUIElement::offsetChildren(int xOffset, int yOffset)
    {
        for (auto& child : children)
        {
            child->xPos += xOffset;
            child->yPos -= yOffset;

            child->offsetChildren(xOffset, yOffset);
        }
    }

    GUIButton::GUIButton(GUIHandler* handler, int xPos, int yPos, int width, int height, bool isMovable, bool isResizable, bool isVisible, bool takesInput, int borderWidth, int cornerRadius, glm::vec4 color, 
        std::function<void(GUIButton*)> onClick)
        : GUIElement(handler, xPos, yPos, width, height, isMovable, isResizable, isVisible, takesInput, borderWidth, cornerRadius, color), onClick(onClick) 
    {
        if (onClick == nullptr)
        {
            std::cerr << "Null onClick function passed to GUIButton constructor, returning early" << std::endl;
            return;
        }
    }

    GUIButton::~GUIButton() {}

    bool GUIButton::handleInput(const SDL_Event* event, InputState* inputState) 
    {
        if (event->type == SDL_MOUSEBUTTONUP) 
        {
            if (event->button.button == SDL_BUTTON_LEFT) 
            {
                int mouseX = event->button.x;
                int mouseY = (int)handler->getWindowHeight() - event->button.y; // Conversion from top-left to bottom-left system
                if (mouseX >= xPos && mouseX < xPos + width && mouseY >= yPos && mouseY < yPos + height) 
                {
                    onClick(this);
                    return true;
                }
            }
        }

        return false;
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
        button->handler->publish(new event::QuitEvent());
    }

    GUIText::GUIText(GUIHandler* handler, int xPos, int yPos, int width, int height, bool isMovable, bool isResizable, bool isVisible, bool takesInput, int borderWidth, int cornerRadius, glm::vec4 color,
        std::wstring text, text::Font* font, bool autoScaleText, float textScale, int padding)
        : GUIElement(handler, xPos, yPos, width, height, isMovable, isResizable, isVisible, takesInput, borderWidth, cornerRadius, color), text(text), font(font), autoScaleText(autoScaleText), textScale(textScale), padding(padding)
    {
        if (font == nullptr)
        {
            std::cerr << "Null font passed to GUIText constructor, returning early" << std::endl;
            return;
        }

        if (autoScaleText == true && textScale != 1.0f)
        {
            std::cerr << "autoScaleText set to true but textScale != 1.0f, textScale reverted to 1.0f" << std::endl;
            textScale = 1.0f;
        }

        if (padding > std::min(width, height) / 2)
        {
            std::cout << "Warning: GUIText created with padding larger than half of minimum span of GUIElement, meaning text will not be visible" << std::endl;
        }
        else if (padding < 0.0f)
        {
            std::cout << "Warning: GUIText created with negative padding, possibility of text ending up outside of GUIElement, thus not being rendered" << std::endl;
        }

        characters = text::createText(text, font);

        if (!initFontTextures())
        {
            std::cerr << "Failed to load textures for a font belonging to text provided to GUIText" << std::endl;
        }
    }

    // TODO: Do something with Font pointer?
    GUIText::~GUIText() 
    {
        for (auto& pair : fontTextures)
        {
            for (GLuint textureID : pair.second)
            {
                glDeleteTextures(1, &textureID);
            }
        }

        // OpenGL cleanup
        cleanupBuffers();

        glDeleteProgram(shaderProgram);
    }

    void GUIText::onResize()
    {
        cleanupBuffers();
        initializeBuffers();
    }

    const char* GUIText::getVertexShader()
    {
        return shaders::textVertexShaderSource;
    }

    const char* GUIText::getFragmentShader()
    {
        return shaders::textFragmentShaderSource;
    }

    bool GUIText::initializeShaders()
    {
        shaderProgram = shaders::createShaderProgram(getVertexShader(), getFragmentShader());
        if (shaderProgram == 0)
        {
            return false;
        }

        // Get uniform locations necessary for GUIText rendering
        projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        modelLoc = glGetUniformLocation(shaderProgram, "model");
        textLoc = glGetUniformLocation(shaderProgram, "text");
        textColorLoc = glGetUniformLocation(shaderProgram, "textColor");

        return true;
    }

    // Cursor from x,y = 0 (or padding), pass cursors for each character to calculateVertices
    bool GUIText::initializeBuffers()
    {
        totalWidth = 0, totalHeight = 0;
        lines = text::createLines(characters, &totalWidth, &totalHeight);

        float scaleX = (width - padding * 2) / totalWidth;
        float scaleY = (height - padding * 2) / totalHeight;

        if (autoScaleText)
        {
            textScale = std::min(scaleX, scaleY);
        }

        float xCursor = (float)padding, yCursor = -(float)padding; // Initialize x and y to the starting position of the text
        // For each line, traverse characters and calculate vertices
        for (size_t i = 0; i < lines.size(); ++i)
        {
            xCursor = (float)padding;
            lines[i]->startX = xCursor;
            for (const auto& ch : lines[i]->characters)
            {
                std::vector<float> vertices = calculateVertices(ch, xCursor, yCursor);

                // Create a VAO for this character
                GLuint VAO;
                glGenVertexArrays(1, &VAO);
                glBindVertexArray(VAO);

                // Create a VBO for this character
                GLuint VBO;
                glGenBuffers(1, &VBO);
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

                // Set up the vertex attributes
                glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(0);

                characterVAOs.emplace_back(VAO);

                // Clean up
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindVertexArray(0);

                // Advance cursor for next glyph
                xCursor += ch->advance * ch->font->getSize() * textScale;
            }

            lines[i]->endX = xCursor;
            lines[i]->yPosition = yCursor;

            yCursor -= lines[i]->height * textScale;
        }

        // Check for OpenGL errors
        GLenum error = glGetError();
        if (error != GL_NO_ERROR)
        {
            std::cerr << "Error when initializing GUIText buffers, OpenGL error: " << error << std::endl;
            return false;
        }

        return true;
    }

    void GUIText::prepareTextRendering() const
    {
        glUseProgram(shaderProgram);

        // Set the scissor rectangle to the bounding box of the GUIElement
        glEnable(GL_SCISSOR_TEST);
        glScissor(xPos, yPos, width, height);

        glm::mat4 projection = glm::ortho(0.0f, handler->getWindowWidth(), 0.0f, handler->getWindowHeight());
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glUniform1i(textLoc, 0);
        glUniform4fv(textColorLoc, 1, glm::value_ptr(color));
    }

    void GUIText::finishTextRendering() const
    {
        glDisable(GL_SCISSOR_TEST);

        // Clean up
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    bool GUIText::render() const
    {
        prepareTextRendering();

        int charVAOIx = -1;
        float yLineOffset = height - lines[0]->maxAscender * textScale;
        // Bind the VAO and texture for each character and draw it
        for (size_t i = 0; i < lines.size(); ++i)
        {
            for (size_t j = 0; j < lines[i]->characters.size(); j++)
            {
                ++charVAOIx;
                text::Character* ch = lines[i]->characters[j];

                if (!shouldRenderCharacter(charVAOIx))
                {
                    continue;
                }

                // Bind the texture for this character
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, fontTextures.at(ch->font).at(0));

                // Create the model matrix for this character
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(xPos, yPos + yLineOffset, 0.0f));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

                // Bind the VAO for this character and draw it
                glBindVertexArray(characterVAOs[charVAOIx]);
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }
        }

        finishTextRendering();

        // Check for OpenGL errors
        GLenum error = glGetError();
        if (error != GL_NO_ERROR)
        {
            std::cerr << "Error when rendering GUIText, OpenGL error: " << error << std::endl;
            return false;
        }

        // Render children
        renderChildren();

        return true;
    }

    void GUIText::cleanupBuffers()
    {
        for (GLuint VAO : characterVAOs)
        {
            glDeleteVertexArrays(1, &VAO);
        }
        characterVAOs.clear();
    }

    bool GUIText::shouldRenderCharacter(int charVAOIx) const
    {
        return true;
    }

    bool GUIText::initFontTextures()
    {
        std::vector<text::Font*> fonts;
        if (characters.empty())
        {
            fonts.emplace_back(font);
        }
        else 
        {
            for (text::Character* ch : characters)
            {
                if (std::find(fonts.begin(), fonts.end(), ch->font) == fonts.end())
                {
                    fonts.emplace_back(ch->font);
                }
            }
        }

        for (text::Font* font : fonts) 
        {
            std::vector<GLuint> textureIDs;
            for (const auto& texturePath : font->getPngPaths()) 
            {
                // Load the texture from the file and store the texture ID
                GLuint textureID = texturel::loadFontTexture(texturePath);
                if (textureID == 0)
                {
                    std::cerr << "Failed to load texture from: " << texturePath << std::endl;
                    return false;
                }

                textureIDs.push_back(textureID);
            }

            fontTextures[font] = textureIDs;
        }

        return true;
    }

    std::vector<float> GUIText::calculateVertices(text::Character* ch, float x, float y)
    {
        float fontSize = ch->font->getSize();

        // Compute the absolute pixel bounds of the glyph
        float x1 = x + ch->planeLeft * fontSize * textScale;
        float y1 = y + ch->planeBottom * fontSize * textScale;
        float x2 = x + ch->planeRight * fontSize * textScale;
        float y2 = y + ch->planeTop * fontSize * textScale;

        // Compute the texture coordinates of the glyph
        float u1 = ch->atlasLeft / ch->font->getTextureWidth();
        float v1 = ch->atlasBottom / ch->font->getTextureHeight();
        float u2 = ch->atlasRight / ch->font->getTextureWidth();
        float v2 = ch->atlasTop / ch->font->getTextureHeight();

        std::vector<float> retVec = {
            // Triangle 1
            x1, y1, u1, v1,
            x2, y1, u2, v1,
            x1, y2, u1, v2,
            // Triangle 2
            x1, y2, u1, v2,
            x2, y1, u2, v1,
            x2, y2, u2, v2
        };

        return retVec;
    }

    GUIEditText::GUIEditText(GUIHandler* handler, int xPos, int yPos, int width, int height, bool isMovable, bool isResizable, bool isVisible, int borderWidth, int cornerRadius, glm::vec4 color,
        std::wstring text, text::Font* font, bool autoScaleText, float textScale, int padding)
        : GUIText(handler, xPos, yPos, width, height, isMovable, isResizable, isVisible, true, borderWidth, cornerRadius, color, text, font, autoScaleText, textScale, padding) {}

    GUIEditText::~GUIEditText() {}

    // TODO: Maybe change text editing to start from specific char clicked?
    // add ctrl+c, ctrl+v, ctrl+z, ctrl+y, add cursor after current character

    // TODO: Bug with beingEdited not being changed back when the element is no longer active as a 
    // result of the activeElement being changed elsewhere in code without changing beingEdited too
    bool GUIEditText::handleInput(const SDL_Event* event, InputState* inputState)
    {
        switch (event->type)
        {
            case SDL_TEXTINPUT:
                if (beingEdited)
                {
                    text += cStringToWString(event->text.text);
                    if (!regenCharactersAndBuffers())
                    {
                        std::cerr << "Failed to regenerate characters and buffers for GUIEditText in handleInput" << std::endl;
                    }
                    return true;  // Event consumed
                }
                break;

            case SDL_KEYDOWN:
                if (beingEdited)
                {
                    // Handle special keys like backspace or enter
                    switch (event->key.keysym.sym)
                    {
                        case SDLK_BACKSPACE:
                            if (!text.empty())
                            {
                                if (text.back() == '\n')
                                {
                                    text.pop_back();
                                    text.pop_back();
                                }
                                else
                                {
                                    text.pop_back();
                                }

                                if (!regenCharactersAndBuffers())
                                {
                                    std::cerr << "Failed to regenerate characters and buffers for GUIEditText in handleInput" << std::endl;
                                }
                            }
                            return true;  // Event consumed
                            break;

                        case SDLK_RETURN:
                            text += '\n';
                            if (!regenCharactersAndBuffers())
                            {
                                std::cerr << "Failed to regenerate characters and buffers for GUIEditText in handleInput" << std::endl;
                            }
                            return true;  // Event consumed
                            break;

                        case SDLK_ESCAPE:
                            stopTextInput(inputState);
                            return true;  // Event consumed
                            break;

                        default:
                            break;
                    }
                }
                break;

            case SDL_MOUSEBUTTONUP:
                if (event->button.button == SDL_BUTTON_LEFT)
                {
                    int mouseX = event->button.x;
                    int mouseY = (int)handler->getWindowHeight() - event->button.y; // Conversion from top-left to bottom-left system
                    if (isOnElement(mouseX, mouseY))
                    {
                        if (text.empty())
                        {
                            startTextInput(inputState);
                        }
                        else if (isOnText(mouseX, mouseY))
                        {
                            startTextInput(inputState);
                        }
                        else
                        {
                            stopTextInput(inputState);
                        }
                        return true;  // Event consumed
                    }
                    else
                    {
                        stopTextInput(inputState);
                        return true;  // Event consumed
                    }
                }
                break;

            default:
                break;
        }

        return false;
    }

    void GUIEditText::setBeingEdited(bool beingEdited)
    {
        this->beingEdited = beingEdited;
    }

    bool GUIEditText::shouldRenderCharacter(int charVAOIx) const
    {
        auto now = std::chrono::steady_clock::now();
        // Ensure that if not beingEdited then lastCharacterVisible is true
        if (!beingEdited)
        {
            lastCharacterVisible = true;
        }
        else if (beingEdited && (now - lastBlinkTime > blinkDuration))
        {
            lastCharacterVisible = !lastCharacterVisible;
            lastBlinkTime = now;
        }

        if (charVAOIx == characterVAOs.size() - 1 && !lastCharacterVisible) 
        {
            return false;
        }

        return true;
    }

    // TODO: Take control of activeElement and the keyboard
    void GUIEditText::startTextInput(InputState* inputState)
    {
        handler->setActiveElement(this);
        beingEdited = true;

        inputState->setKeyboardState(InputState::KeyboardState::GUIKeyboardControl);
        SDL_StartTextInput();
    }

    // TODO: Resign control of activeElement and the keyboard
    void GUIEditText::stopTextInput(InputState* inputState)
    {
        handler->setActiveElement(nullptr);
        beingEdited = false;

        inputState->setKeyboardState(InputState::KeyboardState::MovementControl);
        SDL_StopTextInput();
    }

    bool GUIEditText::regenCharactersAndBuffers()
    {
        // Ensure the last character of whatever Character
        // sequence is generated will be visible
        lastCharacterVisible = true;
        lastBlinkTime = std::chrono::steady_clock::now();

        characters = text::createText(text, font);

        // Regenerate Characters and buffers
        cleanupBuffers();
        return initializeBuffers();
    }

    // TODO: Conversion deprecated
    std::wstring GUIEditText::cStringToWString(const char* inputText)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        std::wstring retText;
    
        try 
        {
            std::wstring wInputText = converter.from_bytes(inputText);
            retText += wInputText;
        } 
        catch (std::range_error& e) 
        {
            std::cerr << "Error when converting string: " << e.what() << '\n';
            return false;
        }

        return retText;
    }

    bool GUIEditText::isOnText(int x, int y)
    {
        for (const auto& line : lines)
        {
            if (isOnLine(line, x, y))
            {
                return true;
            }
        }

        return false;
    }

    // TODO: Not precise for small text, bound stretches above text into text above
    bool GUIEditText::isOnLine(text::Line* line, int x, int y)
    {
        int startBoundX = xPos + (int)line->startX;
        int endBoundX = xPos + (int)line->endX;
        int topBoundY = yPos + height + (int)line->yPosition;
        int lowerBoundY = topBoundY - (int)(line->height * textScale);

        if (x >= startBoundX && x <= endBoundX && y <= topBoundY && y >= lowerBoundY)
        {
            return true;
        }

        return false;
    }

    // TODO: I don't know if this works
    bool GUIEditText::isOnCharacterInLine(text::Character* ch, text::Line* line, int x, int y)
    {
        float xCursor = 0;
        for (const auto& lineCh : line->characters)
        {
            if (lineCh == ch)
            {
                float startBoundX = line->startX * textScale + xCursor;
                float endBoundX = startBoundX + lineCh->advance * ch->font->getSize() * textScale;
                float topBoundY = line->yPosition * textScale;
                float lowerBoundY = topBoundY - line->height * textScale;

                if (x >= startBoundX && x <= endBoundX && y >= lowerBoundY && y <= topBoundY)
                {
                    return true;
                }
            }
            xCursor += lineCh->advance * ch->font->getSize() * textScale;
        }

        return false;
    }

    GUIElementBuilder& GUIElementBuilder::setHandler(GUIHandler* handler) {
        this->handler = handler;
        return *this;
    }

    GUIElementBuilder& GUIElementBuilder::setPosition(int xPos, int yPos) {
        this->xPos = xPos;
        this->yPos = yPos;
        if (xPos < 0 || yPos < 0)
        {
            std::cout << "Warning: Negative position passed to GUIElementBuilder setPosition, possibility of element ending up outside of visible space" << std::endl;
        }
        return *this;
    }

    GUIElementBuilder& GUIElementBuilder::setSize(int width, int height) {
        this->width = width;
        this->height = height;
        if (width <= 0 || height <= 0)
        {
            std::cout << "Warning: Non-positive size passed to GUIElementBuilder setSize, possibility of element not being visible" << std::endl;
        }
        return *this;
    }

    GUIElementBuilder& GUIElementBuilder::setFlags(bool isMovable, bool isResizable, bool isVisible, bool takesInput) {
        this->isMovable = isMovable;
        this->isResizable = isResizable;
        this->isVisible = isVisible;
        this->takesInput = takesInput;
        if (isMovable && !takesInput)
        {
            std::cout << "GUIElementBuilder setFlags called with isMovable = true but takesInput = false, takesInput set to TRUE" << std::endl;
            this->takesInput = true;
        }
        else if (isResizable && !takesInput)
        {
            std::cout << "GUIElementBuilder setFlags called with isResizable = true but takesInput = false, takesInput set to TRUE" << std::endl;
            this->takesInput = true;
        }
        return *this;
    }

    GUIElementBuilder& GUIElementBuilder::setEdgeParameters(int borderWidth, int cornerRadius) {
        this->borderWidth = borderWidth;
        this->cornerRadius = cornerRadius;
        if (borderWidth < 0)
        {
            std::cout << "Warning: Negative borderWidth passed to GUIElementBuilder setEdgeParameters" << std::endl;
        }
        if (cornerRadius < 0)
        {
            std::cout << "Warning: Negative cornerRadius passed to GUIElementBuilder setEdgeParameters" << std::endl;
        }
        return *this;
    }

    GUIElementBuilder& GUIElementBuilder::setColor(glm::vec4 color) {
        this->color = color;
        if (color.a = 0)
        {
            std::cout << "Warning: Color with alpha = 0 passed to GUIElementBuilder setColor, possibility of element not being visible" << std::endl;
        }
        return *this;
    }

    GUIElement* GUIElementBuilder::buildElement() {
        GUIElement* element = new GUIElement(handler, xPos, yPos, width, height, isMovable, isResizable, isVisible, takesInput, borderWidth, cornerRadius, color);
        if (element->initializeShaders() && element->initializeBuffers()) 
        {
            return element;
        } 
        else 
        {
            std::cerr << "Failed to create GUIElement" << std::endl;
            delete element;
            return nullptr;
        }
    }

    GUIElementBuilder& GUIElementBuilder::setOnClick(std::function<void(GUIButton*)> onClick) {
        if (onClick == nullptr)
        {
            std::cout << "Null onClick function passed to GUIElementBuilder setOnClick, new onClick not set, retaining its current value" << std::endl;
            return *this;
        }
        this->onClick = onClick;
        return *this;
    }

    GUIButton* GUIElementBuilder::buildButton() {
        GUIButton* buttonElement = new GUIButton(handler, xPos, yPos, width, height, isMovable, isResizable, isVisible, takesInput, borderWidth, cornerRadius, color, onClick);
        if (buttonElement->initializeShaders() && buttonElement->initializeBuffers()) 
        {
            return buttonElement;
        } 
        else 
        {
            std::cerr << "Failed to create GUIButton" << std::endl;
            delete buttonElement;
            return nullptr;
        }
    }

    GUIElementBuilder& GUIElementBuilder::setText(std::wstring text) {
        this->text = text;
        return *this;
    }

    GUIElementBuilder& GUIElementBuilder::setFont(text::Font* font) {
        if (font == nullptr)
        {
            std::cout << "Null font passed to GUIElementBuilder setFont, new font not set, retaining its current value" << std::endl;
            return *this;
        }
        this->font = font;
        return *this;
    }

    GUIElementBuilder& GUIElementBuilder::setAutoScaleText(bool autoScaleText) {
        this->autoScaleText = autoScaleText;
        return *this;
    }

    GUIElementBuilder& GUIElementBuilder::setTextScale(float textScale) {
        this->textScale = textScale;
        return *this;
    }

    GUIElementBuilder& GUIElementBuilder::setPadding(int padding) {
        this->padding = padding;
        return *this;
    }

    GUIText* GUIElementBuilder::buildText() {
        GUIText* textElement = new GUIText(handler, xPos, yPos, width, height, isMovable, isResizable, isVisible, takesInput, borderWidth, cornerRadius, color, text, font, autoScaleText, textScale, padding);
        if (textElement->initializeShaders() && textElement->initializeBuffers()) 
        {
            return textElement;
        } 
        else 
        {
            std::cerr << "Failed to create GUIText" << std::endl;
            delete textElement;
            return nullptr;
        }
    }

    GUIEditText* GUIElementBuilder::buildEditText() {
        GUIEditText* editTextElement = new GUIEditText(handler, xPos, yPos, width, height, isMovable, isResizable, isVisible, borderWidth, cornerRadius, color, text, font, autoScaleText, textScale, padding);
        if (editTextElement->initializeShaders() && editTextElement->initializeBuffers()) 
        {
            return editTextElement;
        } 
        else 
        {
            std::cerr << "Failed to create GUIEditText" << std::endl;
            delete editTextElement;
            return nullptr;
        }
    }
}