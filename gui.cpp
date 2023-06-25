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
    // TODO: Use pub-sub to communicate changes in
    // windowWidth and windoHeight in renderer
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
            for (auto& e : elements)
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

    bool GUIHandler::handleGUIElementInput(GUIElement* element, const SDL_Event* event, InputState* inputState)
    {
        if (elements.find(element) == elements.end())
        {
            std::cerr << "Element to handle input for not present in GUIHandler, unable to handle input for element" << std::endl;
            return false;
        }

        return element->handleInput(event, inputState);
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

    bool GUIHandler::renderWholeVector() const
    {
        prepareGUIRendering();

        bool result = true;
        for (auto& e : renderVector)
        {
            if (!renderGUIElement(e))
            {
                std::cerr << "Issue rendering individual GUIElement when rendering all elements in renderVector" << std::endl;
                result = false;
            }
        }

        finishGUIRendering();

        return result;
    }

    bool GUIHandler::handleInputWholeVector(const SDL_Event* event, InputState* inputState)
    {
        bool result = true;
        for (auto& e : handleInputVector)
        {
            if(!handleGUIElementInput(e, event, inputState))
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

        // Check for OpenGL errors
        GLenum error = glGetError();
        if (error != GL_NO_ERROR)
        {
            std::cerr << "Error when rendering GUIElement, OpenGL error: " << error << std::endl;
            return false;
        }

        return true;
    }

    bool GUIElement::handleInput(const SDL_Event* event, InputState* inputState)
    {
        // Only GUIElement that isMovable should be movable
        if (isMovable)
        {
            move(event, inputState);
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

    void GUIElement::move(const SDL_Event* event, InputState* inputState)
    {
        switch (event->type)
        {
        case SDL_MOUSEBUTTONDOWN:
            if (event->button.button == SDL_BUTTON_LEFT)
            {
                int mouseX = event->button.x;
                int mouseY = (int)(handler->getWindowHeight() - event->button.y); // Conversion from top-left to bottom-left system
                if (mouseX >= xPos && mouseX <= xPos + width && mouseY >= yPos && mouseY <= yPos + height)
                {
                    isBeingDragged = true;
                    inputState->setMouseState(InputState::GUIMouseControl);
                }
            }
            break;

        case SDL_MOUSEBUTTONUP:
            if (event->button.button == SDL_BUTTON_LEFT)
            {
                isBeingDragged = false;
                inputState->setMouseState(InputState::CameraControl);
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
        for (auto& child : children)
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

    bool GUIButton::handleInput(const SDL_Event* event, InputState* inputState) 
    {
        if (isMovable)
        {
            move(event, inputState);
        }

        if (event->type == SDL_MOUSEBUTTONUP) 
        {
            if (event->button.button == SDL_BUTTON_LEFT) 
            {
                int mouseX = event->button.x;
                int mouseY = (int)(handler->getWindowHeight() - event->button.y); // Conversion from top-left to bottom-left system
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

    // TODO: Implement publish-subscribe system to communicate between
    // renderer and gui, can here allow to publish "quit" notification
    // to be passed to renderer which can then quit renderer
    void GUIButton::quitApplication(GUIButton* button)
    {
        std::cerr << "Quit application button method not implemented" << std::endl;
    }

    GUIText::GUIText(GUIHandler* handler, int xPos, int yPos, int width, int height, glm::vec4 color,
        std::wstring text, text::Font* font, bool autoScaleText, float textScale, bool isMovable, bool isVisible, bool takesInput)
        : GUIElement(handler, xPos, yPos, width, height, color, isMovable, isVisible, takesInput), text(text), font(font), autoScaleText(autoScaleText), textScale(textScale)
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

    bool GUIText::initializeBuffers()
    {
        baseline = 0;

        // Iterate through all characters to find the maximum yOffset
        for (auto& pair : font->getIdCharacterMap()) 
        {
            if (pair.second.yOffset > baseline) 
            {
                baseline = (float)pair.second.yOffset;
            }
        }

        totalWidth = 0, totalHeight = 0;
        lines = text::createLines(characters, &totalWidth, &totalHeight);

        float scaleX = width / totalWidth;
        float scaleY = height / totalHeight;

        if (autoScaleText)
        {
            textScale = std::min(scaleX, scaleY);
        }

        float xCursor = 0, yCursor = 0; // Initialize x and y to the starting position of the text
        // For each line, traverse characters and calculate vertices
        for (size_t i = 0; i < lines.size(); ++i)
        {
            xCursor = 0;
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
                xCursor += ch->xAdvance * textScale;
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

    void GUIText::cleanupBuffers()
    {
        for (GLuint VAO : characterVAOs)
        {
            glDeleteVertexArrays(1, &VAO);
        }
        characterVAOs.clear();
    }

    bool GUIText::render() const
    {
        prepareTextRendering();

        int charVAOIx = -1;
        float yLineOffset = height - (lines[0]->height * textScale);
        // Bind the VAO and texture for each character and draw it
        for (size_t i = 0; i < lines.size(); ++i)
        {
            for (size_t j = 0; j < lines[i]->characters.size(); j++)
            {
                ++charVAOIx;
                text::Character* ch = lines[i]->characters[j];

                // Bind the texture for this character
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, fontTextures.at(ch->font).at(ch->page));

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

        return true;
    }

    bool GUIText::loadFontTextures()
    {
        std::vector<text::Font*> fonts;
        for (text::Character* ch : characters)
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

                if (font->getTextureNbrChannels() == 3)
                {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)font->getTextureWidth(), (GLsizei)font->getTextureHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, texture);
                }
                else if (font->getTextureNbrChannels() == 4)
                {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)font->getTextureWidth(), (GLsizei)font->getTextureHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, texture);
                }

                glGenerateMipmap(GL_TEXTURE_2D);
                
                textureIDs.push_back(textureID);
            }

            fontTextures[font] = textureIDs;
        }

        return true;
    }

    std::vector<float> GUIText::calculateVertices(text::Character* ch, float x, float y)
    {
        float xpos = x + ch->xOffset * textScale;
        float ypos = y + (baseline - ch->yOffset - ch->height) * textScale;
        
        float w = ch->width * textScale;
        float h = ch->height * textScale;

        // Calculate the texture coordinates for this character
        float xTex = ch->x / font->getTextureWidth();
        float yTex = (ch->y + ch->height) / font->getTextureHeight();
        float wTex = ch->width / font->getTextureWidth();
        float hTex = -ch->height / font->getTextureHeight();

        std::vector<float> retVec = {
            xpos,     (ypos + h), xTex,        yTex + hTex,
            xpos,     ypos,       xTex,        yTex,
            xpos + w, ypos,       xTex + wTex, yTex,

            xpos,     (ypos + h), xTex,        yTex + hTex,
            xpos + w, ypos,       xTex + wTex, yTex,
            xpos + w, (ypos + h), xTex + wTex, yTex + hTex
        };

        return retVec;
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

    GUIEditText::GUIEditText(GUIHandler* handler, int xPos, int yPos, int width, int height, glm::vec4 color,
        std::wstring text, text::Font* font, bool autoScaleText, float textScale, bool isMovable, bool isVisible)
        : GUIText(handler, xPos, yPos, width, height, color, text, font, autoScaleText, textScale, isMovable, isVisible, true)
    {
        beingEdited = false;
        lastCharacterVisible = true;
        lastBlinkTime = std::chrono::steady_clock::now();
    }

    GUIEditText::~GUIEditText() {}

    bool GUIEditText::render() const
    {
        prepareTextRendering();

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

        int charVAOIx = -1;
        float yLineOffset = height - (lines[0]->height * textScale);
        // Bind the VAO and texture for each character and draw it
        for (size_t i = 0; i < lines.size(); ++i)
        {
            for (size_t j = 0; j < lines[i]->characters.size(); j++)
            {
                ++charVAOIx;
                text::Character* ch = lines[i]->characters[j];

                if (charVAOIx == characterVAOs.size() - 1 && !lastCharacterVisible) 
                {
                    continue;
                }

                // Bind the texture for this character
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, fontTextures.at(ch->font).at(ch->page));

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

        return true;
    }

    // TODO: Maybe change text editing to start from specific char clicked?
    // add ctrl+c, ctrl+v, ctrl+z, ctrl+y, add cursor after current character
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

        // Regenerate buffers
        cleanupBuffers();
        return initializeBuffers();
    }

    // Conversion deprecated
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

    // TODO: Not precise for small text, bound stretches above text into text above
    bool GUIEditText::isOnCharacterInLine(text::Character* ch, text::Line* line, int x, int y)
    {
        int xCursor = 0;
        for (const auto& lineCh : line->characters)
        {
            if (lineCh == ch)
            {
                int startBoundX = xPos + (int)line->startX + xCursor;
                int endBoundX = startBoundX + (int)(lineCh->width * textScale);
                int topBoundY = yPos + height + (int)line->yPosition;
                int lowerBoundY = topBoundY - (int)(line->height * textScale);

                if (x >= startBoundX && x <= endBoundX && y <= topBoundY && y >= lowerBoundY)
                {
                    return true;
                }
            }
            xCursor += (int)(lineCh->xAdvance * textScale);
        }

        return false;
    }

    GUIElement* GUIElementFactory::createGUIElement(GUIHandler* handler, int xPos, int yPos, int width, int height, glm::vec4 color, bool isMovable, bool isVisible, bool takesInput) 
    {
        GUIElement* element = new GUIElement(handler, xPos, yPos, width, height, color, isMovable, isVisible, takesInput);
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

    GUIButton* GUIElementFactory::createGUIButton(GUIHandler* handler, int xPos, int yPos, int width, int height, glm::vec4 color, std::function<void(GUIButton*)> onClick, bool isMovable, bool isVisible, bool takesInput) 
    {
        GUIButton* buttonElement = new GUIButton(handler, xPos, yPos, width, height, color, onClick, isMovable, isVisible, takesInput);
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

    GUIText* GUIElementFactory::createGUIText(GUIHandler* handler, int xPos, int yPos, int width, int height, glm::vec4 color, std::wstring text, text::Font* font, bool autoScaleText, float textScale, bool isMovable, bool isVisible, bool takesInput) 
    {
        GUIText* textElement = new GUIText(handler, xPos, yPos, width, height, color, text, font, autoScaleText, textScale, isMovable, isVisible, takesInput);
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

    GUIEditText* GUIElementFactory::createGUIEditText(GUIHandler* handler, int xPos, int yPos, int width, int height, glm::vec4 color, std::wstring text, text::Font* font, bool autoScaleText, float textScale, bool isMovable, bool isVisible) 
    {
        GUIEditText* editTextElement = new GUIEditText(handler, xPos, yPos, width, height, color, text, font, autoScaleText, textScale, isMovable, isVisible);
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