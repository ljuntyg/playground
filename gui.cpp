#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <random>
#include <locale>
#include <codecvt>

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
        if (!rootElements.empty())
        {
            for (auto& e : rootElements)
            {
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

    void GUIHandler::addElement(GUIElement* element)
    {
        rootElements.emplace(element);
    }

    void GUIHandler::removeElement(GUIElement* element)
    {
        auto it = std::find(rootElements.begin(), rootElements.end(), element);
        if(it == rootElements.end())
        {
            std::cerr << "Element not a member of elements, unable to remove from elements" << std::endl;
            return;
        }

        rootElements.erase(it);
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
        for (auto& e : rootElements)
        {
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
        bool result = true;
        for (auto& e : rootElements)
        {
            if (e->getTakesInput())
            {
                if (!e->receiveInput(event, inputState))
                {
                    std::cerr << "Issue handling input for individual GUIElement when handling input for all elements in GUIHandler" << std::endl;
                    result = false;
                }
            }
        }

        return result;
    }

    GUIElement::GUIElement(GUIHandler* handler, int xPos, int yPos, int width, int height, bool isMovable, bool isResizable, bool isVisible, bool takesInput, int borderWidth, glm::vec4 color)
        : handler(handler), xPos(xPos), yPos(yPos), width(width), height(height), isMovable(isMovable), isResizable(isResizable), isVisible(isVisible), takesInput(takesInput), borderWidth(borderWidth), color(color)
    {
        // TODO: Ensure xPos, yPos, width and height places element within screen (width and height clip outside screen?), also needs to be checked on handleInput/render
        if (isMovable && !takesInput)
        {
            std::cerr << "Attempt to create movable GUIElement that does not take input, element made immovable" << std::endl;
            isMovable = false;
        }

        if (isResizable && !takesInput)
        {
            std::cerr << "Attempt to create resizable GUIElement that does not take input, element made non-resizable" << std::endl;
            isResizable = false;
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
        return true;
    }

    // TODO: Handle corners for children offset outside parent
    bool GUIElement::receiveInput(const SDL_Event* event, InputState* inputState)
    {
        if (isResizable)
        {
            resize(event, inputState);
            onResize();
        }
        if (isMovable)
        {
            move(event, inputState);
        }

        // Define an early return condition, where return if a manipulation has happened,
        // i.e. if manipulationState was non-None before AND is None after
        // AND a manipulation-relevant event has occurred (moved OR size changed)
        if ((lastManipulationState != ElementManipulationState::None)
            && (manipulationState == ElementManipulationState::None)
            && (lastManipulationState == ElementManipulationState::WasDragged 
                || lastManipulationState == ElementManipulationState::WasResized))
        {
            return true;
        }

        handleInput(event, inputState);

        lastManipulationState = manipulationState;
        return receiveInputChildren(event, inputState);
    }

    bool GUIElement::receiveInputChildren(const SDL_Event* event, InputState* inputState)
    {
        for (auto& child : children)
        {
            if (!child->receiveInput(event, inputState))
            {
                std::cerr << "Issue handling input for individual GUIElement when handling input for children" << std::endl;
                return false;
            }
        }

        return true;
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

    ElementManipulationState GUIElement::getManipulationState()
    {
        return manipulationState;
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

    void GUIElement::resize(const SDL_Event* event, InputState* inputState)
    {
        // Either resize or move, not at the same time
        if (manipulationState == ElementManipulationState::BeingDragged)
        {
            return;
        }

        switch (event->type)
        {
        case SDL_MOUSEBUTTONDOWN:
            if (event->button.button == SDL_BUTTON_LEFT)
            {
                int mouseX = event->button.x;
                int mouseY = (int)(handler->getWindowHeight() - event->button.y); // Conversion from top-left to bottom-left system
                if (isOnAnyCorner(mouseX, mouseY, &cornerNbrBeingResized))
                {
                    manipulationState = ElementManipulationState::BeingResized;
                    inputState->setMouseState(InputState::GUIResizeControl);
                }
            }
            break;

        case SDL_MOUSEBUTTONUP:
            if (event->button.button == SDL_BUTTON_LEFT)
            {
                manipulationState = ElementManipulationState::None;
                inputState->setMouseState(InputState::CameraControl);
            }
            break;

        case SDL_MOUSEMOTION:
            if (manipulationState == ElementManipulationState::BeingResized
                || manipulationState == ElementManipulationState::WasResized)
            {
                manipulationState = ElementManipulationState::WasResized;

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
                    }
                    break;

                case 2: // Top Right
                    if(width + xOffset >= 2 * borderWidth && height + yOffset >= 2 * borderWidth)
                    {
                        width += xOffset;
                        height += yOffset;
                        // offsetChildren(0, 0);
                        resizeChildren(xOffset, yOffset);
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
                    }
                    break;
                    
                default:
                    break;
                }
            }
            break;
        }
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

    void GUIElement::move(const SDL_Event* event, InputState* inputState)
    {
        // Either resize or move, not at the same time
        if (manipulationState == ElementManipulationState::BeingResized)
        {
            return;
        }

        switch (event->type)
        {
        case SDL_MOUSEBUTTONDOWN:
            if (event->button.button == SDL_BUTTON_LEFT)
            {
                int mouseX = event->button.x;
                int mouseY = (int)handler->getWindowHeight() - event->button.y; // Conversion from top-left to bottom-left system
                if (mouseX >= xPos && mouseX <= xPos + width && mouseY >= yPos && mouseY <= yPos + height)
                {
                    manipulationState = ElementManipulationState::BeingDragged;
                    inputState->setMouseState(InputState::GUIMouseControl);
                }
            }
            break;

        case SDL_MOUSEBUTTONUP:
            if (event->button.button == SDL_BUTTON_LEFT)
            {
                manipulationState = ElementManipulationState::None;
                inputState->setMouseState(InputState::CameraControl);
            }
            break;

        case SDL_MOUSEMOTION:
            if (manipulationState == ElementManipulationState::BeingDragged
                || manipulationState == ElementManipulationState::WasDragged)
            {
                manipulationState = ElementManipulationState::WasDragged;

                int xOffset = event->motion.xrel;
                int yOffset = event->motion.yrel;

                xPos += xOffset;
                yPos -= yOffset;

                offsetChildren(xOffset, yOffset);
            }
            break;
        }
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

    GUIButton::GUIButton(GUIHandler* handler, int xPos, int yPos, int width, int height, bool isMovable, bool isResizable, bool isVisible, bool takesInput, int borderWidth, glm::vec4 color, 
        std::function<void(GUIButton*)> onClick)
        : GUIElement(handler, xPos, yPos, width, height, isMovable, isResizable, isVisible, takesInput, borderWidth, color), onClick(onClick) {}

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
        button->handler->publish(new event::QuitEvent());
    }

    GUIText::GUIText(GUIHandler* handler, int xPos, int yPos, int width, int height, bool isMovable, bool isResizable, bool isVisible, bool takesInput, int borderWidth, glm::vec4 color,
        std::wstring text, text::Font* font, bool autoScaleText, float textScale, int padding)
        : GUIElement(handler, xPos, yPos, width, height, isMovable, isResizable, isVisible, takesInput, borderWidth, color), text(text), font(font), autoScaleText(autoScaleText), textScale(textScale), padding(padding)
    {
        if (font == nullptr)
        {
            std::cerr << "Null font passed to GUIText constructor" << std::endl;
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

        if (!loadFontTextures())
        {
            std::cerr << "Failed to load textures for a font belonging to text provided to GUIText" << std::endl;
        }
    }

    GUIText::~GUIText() 
    {
        // TODO: Do something with Font pointer?
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

        // Get uniform locations
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

    bool GUIText::loadFontTextures()
    {
        std::vector<text::Font*> fonts;
        if (characters.empty())
        {
            fonts.emplace_back(font);
        }
        else for (text::Character* ch : characters)
        {
            if (std::find(fonts.begin(), fonts.end(), ch->font) == fonts.end())
            {
                fonts.emplace_back(ch->font);
            }
        }

        for (text::Font* font : fonts) 
        {
            std::vector<GLuint> textureIDs;
            for (const auto& texture : font->getTextures()) 
            {
                if (!texture)
                {
                    std::cerr << "Invalid texture in loadFontTexture" << std::endl;
                    return false;
                }

                // Load the texture into OpenGL and store the texture ID
                GLuint textureID;
                glGenTextures(1, &textureID);
                glBindTexture(GL_TEXTURE_2D, textureID);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)font->getTextureWidth(), (GLsizei)font->getTextureHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, texture);
                glGenerateMipmap(GL_TEXTURE_2D);
                
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

    GUIEditText::GUIEditText(GUIHandler* handler, int xPos, int yPos, int width, int height, bool isMovable, bool isResizable, bool isVisible, int borderWidth, glm::vec4 color,
        std::wstring text, text::Font* font, bool autoScaleText, float textScale, int padding)
        : GUIText(handler, xPos, yPos, width, height, isMovable, isResizable, isVisible, true, borderWidth, color, text, font, autoScaleText, textScale, padding)
    {
        beingEdited = false;
        lastCharacterVisible = true;
        lastBlinkTime = std::chrono::steady_clock::now();
    }

    GUIEditText::~GUIEditText() {}

    // TODO: Maybe change text editing to start from specific char clicked?
    // add ctrl+c, ctrl+v, ctrl+z, ctrl+y, add cursor after current character
    // Also better handle the cursor blink state, it's a bit buggy
    bool GUIEditText::handleInput(const SDL_Event* event, InputState* inputState)
    {
        // If GUIKeyboardControl but this element not being edited, it means another GUIEditText is being edited
        if (inputState->getKeyboardState() == InputState::KeyboardState::GUIKeyboardControl && !beingEdited)
        {
            return true;
        }

        if (beingEdited)
        {
            if (event->type == SDL_TEXTINPUT)
            {
                text += cStringToWString(event->text.text);
                return regenCharactersAndBuffers();
            }
            else if (event->type == SDL_KEYDOWN)
            {
                // Handle special keys like backspace or enter
                if (event->key.keysym.sym == SDLK_BACKSPACE && !text.empty())
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

                    return regenCharactersAndBuffers();
                }
                else if (event->key.keysym.sym == SDLK_RETURN)
                {
                    text += '\n';
                    return regenCharactersAndBuffers();
                }
                else if (event->key.keysym.sym == SDLK_ESCAPE)
                {
                    stopTextInput(inputState);
                    return true;
                }
            }
        }

        if (event->type == SDL_MOUSEBUTTONUP)
        {
            if (event->button.button == SDL_BUTTON_LEFT)
            {
                if (text.empty())
                {
                    // Seems to work correctly without else statement
                    if (isOnElement(event->button.x, (int)handler->getWindowHeight() - event->button.y))
                    {
                        startTextInput(inputState);
                        return true;
                    }
                }

                if (isOnText(event->button.x, (int)handler->getWindowHeight() - event->button.y))
                {
                    startTextInput(inputState);
                    return true;
                }
                else
                {
                    stopTextInput(inputState);
                    return true;
                }
            }
        }

        return true;
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

    void GUIEditText::startTextInput(InputState* inputState)
    {
        beingEdited = true;

        inputState->setKeyboardState(InputState::KeyboardState::GUIKeyboardControl);
        SDL_StartTextInput();
    }

    void GUIEditText::stopTextInput(InputState* inputState)
    {
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

    GUIElement* GUIElementFactory::createGUIElement(GUIHandler* handler, int xPos, int yPos, int width, int height, bool isMovable, bool isResizable, bool isVisible, bool takesInput, int borderWidth, glm::vec4 color) 
    {
        GUIElement* element = new GUIElement(handler, xPos, yPos, width, height, isMovable, isResizable, isVisible, takesInput, borderWidth, color);
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

    GUIButton* GUIElementFactory::createGUIButton(GUIHandler* handler, int xPos, int yPos, int width, int height, bool isMovable, bool isResizable, bool isVisible, bool takesInput, int borderWidth, glm::vec4 color,
        std::function<void(GUIButton*)> onClick) 
    {
        GUIButton* buttonElement = new GUIButton(handler, xPos, yPos, width, height, isMovable, isResizable, isVisible, takesInput, borderWidth, color, onClick);
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

    GUIText* GUIElementFactory::createGUIText(GUIHandler* handler, int xPos, int yPos, int width, int height, bool isMovable, bool isResizable, bool isVisible, bool takesInput, int borderWidth, glm::vec4 color,
        std::wstring text, text::Font* font, bool autoScaleText, float textScale, int padding) 
    {
        GUIText* textElement = new GUIText(handler, xPos, yPos, width, height, isMovable, isResizable, isVisible, takesInput, borderWidth, color, text, font, autoScaleText, textScale, padding);
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

    GUIEditText* GUIElementFactory::createGUIEditText(GUIHandler* handler, int xPos, int yPos, int width, int height, bool isMovable, bool isResizable, bool isVisible, int borderWidth, glm::vec4 color,
        std::wstring text, text::Font* font, bool autoScaleText, float textScale, int padding) 
    {
        GUIEditText* editTextElement = new GUIEditText(handler, xPos, yPos, width, height, isMovable, isResizable, isVisible, borderWidth, color, text, font, autoScaleText, textScale, padding);
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