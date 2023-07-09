#pragma once

#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <queue>
#include <unordered_set>
#include <functional>   
#include <string>
#include <chrono>
#include <map>

#include "input_state.h"
#include "text.h"
#include "pub_sub.h"

namespace gui
{
    // Forward declarations
    class GUIElement;
    class GUIEditText;
    class GUIElementBuilder;

    const std::unordered_map<std::string, glm::vec4> colorMap = {
        {"RED",        glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)},
        {"GREEN",      glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)},
        {"BLUE",       glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)},
        {"YELLOW",     glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)},
        {"CYAN",       glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)},
        {"MAGENTA",    glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)},
        {"WHITE",      glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)},
        {"BLACK",      glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)},
        {"LIGHT GRAY", glm::vec4(0.75f, 0.75f, 0.75f, 1.0f)},
        {"GRAY",       glm::vec4(0.5f, 0.5f, 0.5f, 1.0f)},
        {"DARK GRAY",  glm::vec4(0.25f, 0.25f, 0.25f, 1.0f)}
    };

    enum class ElementManipulationState
    {
        None,
        PressOnElement,
        PressOnCorner,
        Dragging,
        Resizing
    };
    
    class GUIHandler : public Subscriber, public Publisher
    {
    public:
        GUIHandler(float windowWidth, float windowHeight);
        ~GUIHandler();

        void notify(const event::Event* event) override;

        float getWindowHeight() const;
        float getWindowWidth() const;

        void addElement(GUIElement* element, int zIndex = 0);
        // Be careful about using removeElement externally, better 
        // to access indirectly through "delete element"
        void removeElement(GUIElement* element);

        void prepareGUIRendering() const;
        void finishGUIRendering() const;

        bool renderAllElements() const;
        // receiveInputAllElements has as a side effect that it updates the zIndexRootElementMap
        bool receiveInputAllElements(const SDL_Event* event, InputState* inputState);

        std::multimap<int, GUIElement*> getZIndexRootElementMap();
        void GUIHandler::normalizeZIndices();

        GUIElement* getActiveElement();
        void setActiveElement(GUIElement* element);
        bool isOrContainsActiveElement(GUIElement* element);

    private:
        float windowWidth;
        float windowHeight;

        std::multimap<int, GUIElement*> zIndexRootElementMap;
        GUIElement* activeElement = nullptr;
    };

    class GUIElement 
    {
        friend class GUIElementBuilder;
    public:
        virtual ~GUIElement();

        void addChild(GUIElement* child);
        void removeChild(GUIElement* child);

        virtual bool render() const;
        virtual bool renderChildren() const;

        // Override handleInput for the specific handling of input for each GUIElement subtype,
        // receiveInput is a common wrapper for each subtype's handleInput, handleInput returns
        // a bool indicating whether the event was consumed or not, true for consumed, false for not
        // TODO: Handle corners for children offset outside parent
        virtual bool handleInput(const SDL_Event* event, InputState* inputState);

        // A true return value from receiveInput means the element consumed the event
        bool receiveInput(const SDL_Event* event, InputState* inputState);
        bool receiveInputChildren(const SDL_Event* event, InputState* inputState);

        bool isOnElement(int x, int y);
        bool isOnCorner(int cornerNbr, int x, int y);
        void findPossibleNewCorner(int cornerNbr, int* newX, int* newY);
        // Sets cornerNbrBeingResized to the corner being resized, or 0 if none being resized
        bool isOnAnyCorner(int x, int y, int* cornerNbrBeingResized);

        std::unordered_set<GUIElement*> getChildren();
        int getXPos();
        int getYPos();
        bool getIsMovable();
        bool getIsVisible();
        bool getTakesInput();
        ElementManipulationState getManipulationStateResize();
        ElementManipulationState getManipulationStateMove();

    private:
        // Override to return respective shader for each GUIElement subtype
        virtual const char* getVertexShader(); 
        virtual const char* getFragmentShader();

        virtual bool initializeShaders();
        virtual bool initializeBuffers();
    
    protected:
        // Only constructed through GUIElementBuilder, which defines default values for all parameters
        GUIElement(GUIHandler* handler, int xPos, int yPos, int width, int height, bool isMovable, bool isResizable, bool isVisible, bool takesInput, int borderWidth, int cornerRadius, glm::vec4 color);

        // Override for GUIText text regeneration after resize
        virtual void onResize();

        // Resize and move return true if the event was consumed, false otherwise
        bool resize(const SDL_Event* event, InputState* inputState);
        // Resizes children down to a minSize, any offset past minSize is stored in element accumUnderMinSizeX and accumUnderMinSizeY,
        // when element is upsized again, it checks against accumulations to ensure children are only upsized if they have "made up" for their
        // size underflow, this ensures they are only upsized again when their parent is the same size it was at the point when the minSize was reached
        void resizeChildren(int xOffset, int yOffset);
        bool move(const SDL_Event* event, InputState* inputState);
        void offsetChildren(int xOffset, int yOffset);

        GUIHandler* handler;
        std::unordered_set<GUIElement*> children;

        int xPos, yPos;
        int width, height;
        bool isMovable, isResizable, isVisible, takesInput;
        int borderWidth;
        int cornerRadius; // In pixels, TODO: Add as parameter to constructor
        glm::vec4 color;

        // Manipulation management members
        ElementManipulationState manipulationStateResize = ElementManipulationState::None;
        ElementManipulationState manipulationStateMove = ElementManipulationState::None;
        int cornerNbrBeingResized = 0;
        int accumUnderMinSizeX = 0, accumUnderMinSizeY = 0;

        // Shader stuff
        GLint modelLoc, viewLoc, projectionLoc, useTextureLoc, colorLoc, timeLoc, resolutionLoc, cornerRadiusLoc;
        GLuint shaderProgram, VAO, VBO, EBO;
    };

    class GUIButton : public GUIElement 
    {
        friend class GUIElementBuilder;
    public:
        ~GUIButton() override;

        bool handleInput(const SDL_Event* event, InputState* inputState) override;

        // These are functions that can be passed to the constructor (onClick),
        // must have return type void and take only a GUIButton* as parameter
        static void randomColor(GUIButton* button);
        static void quitApplication(GUIButton* button);
        // Publishes a NextModelEvent which can be handled by the renderer
        static void nextModel(GUIButton* button);
        static void nextCubemap(GUIButton* button);

        static void lightAzimuthUp(GUIButton* button);
        static void lightAzimuthDown(GUIButton* button);
        static void lightInclineUp(GUIButton* button);
        static void lightInclineDown(GUIButton* button);
        static void luminanceUp(GUIButton* button);
        static void luminanceDown(GUIButton* button);
        static void scaleUp(GUIButton* button);
        static void scaleDown(GUIButton* button);
        static void cameraSpeedUp(GUIButton* button);
        static void cameraSpeedDown(GUIButton* button);

    protected:
        // Only constructed through GUIElementBuilder, which defines default values for all parameters
        GUIButton(GUIHandler* handler, int xPos, int yPos, int width, int height, bool isMovable, bool isResizable, bool isVisible, bool takesInput, int borderWidth, int cornerRadius, glm::vec4 color,
            std::function<void(GUIButton*)> onClick);

        std::function<void(GUIButton*)> onClick;
    };

    class GUIText : public GUIElement
    {
        friend class GUIElementBuilder;
    public:
        ~GUIText() override;

        bool render() const override;

    protected:
        // Only constructed through GUIElementBuilder, which defines default values for all parameters
        GUIText(GUIHandler* handler, int xPos, int yPos, int width, int height, bool isMovable, bool isResizable, bool isVisible, bool takesInput, int borderWidth, int cornerRadius, glm::vec4 color,
            std::wstring text, text::Font* font, bool autoScaleText, float textScale, int padding);

        void onResize() override;

        const char* getVertexShader() override; 
        const char* getFragmentShader() override;

        bool initializeShaders() override;
        bool initializeBuffers() override;

        void prepareTextRendering() const;
        void finishTextRendering() const;

        void cleanupBuffers();
        virtual bool shouldRenderCharacter(int charVAOIx) const;
        
        std::wstring text;
        text::Font* font; // I forgot why this is here if each Character has its own Font
        std::vector<text::Character*> characters;
        std::unordered_map<text::Font*, std::vector<GLuint>> fontTextures;
        std::vector<text::Line*> lines;

        int padding;
        float textScale;
        float totalHeight, totalWidth;
        bool autoScaleText;

        // Shader stuff
        std::vector<GLuint> characterVAOs;
        GLint projectionLoc, modelLoc, textLoc, textColorLoc;
    
    private:
        bool initFontTextures();
        std::vector<float> calculateVertices(text::Character* ch, float x, float y);
    };

    class GUIEditText : public GUIText
    {
        friend class GUIElementBuilder;
    public:
        ~GUIEditText() override;

        bool handleInput(const SDL_Event* event, InputState* inputState) override;

        void setBeingEdited(bool beingEdited);

    protected:
        // Only constructed through GUIElementBuilder, which defines default values for all parameters
        GUIEditText(GUIHandler* handler, int xPos, int yPos, int width, int height, bool isMovable, bool isResizable, bool isVisible, int borderWidth, int cornerRadius, glm::vec4 color,
            std::wstring text, text::Font* font, bool autoScaleText, float textScale, int padding);

        bool shouldRenderCharacter(int charVAOIx) const override;

    private:
        void startTextInput(InputState* inputState);
        void stopTextInput(InputState* inputState);

        bool regenCharactersAndBuffers(); 
        std::wstring cStringToWString(const char* inputText);

        bool isOnText(int x, int y);
        bool isOnLine(text::Line* line, int x, int y);
        bool isOnCharacterInLine(text::Character* ch, text::Line* line, int x, int y);

        bool beingEdited = false;
        mutable bool lastCharacterVisible = true;
        mutable std::chrono::steady_clock::time_point lastBlinkTime = std::chrono::steady_clock::now();
        std::chrono::milliseconds blinkDuration = std::chrono::milliseconds(500);
    };

    class GUIElementBuilder {
    public:
        GUIElementBuilder& setHandler(GUIHandler* handler);
        GUIElementBuilder& setPosition(int xPos, int yPos);
        GUIElementBuilder& setSize(int width, int height);
        GUIElementBuilder& setFlags(bool isMovable, bool isResizable, bool isVisible, bool takesInput);
        GUIElementBuilder& setEdgeParameters(int borderWidth, int cornerRadius);
        GUIElementBuilder& setColor(glm::vec4 color);
        GUIElement* buildElement();

        // GUIButton specific
        GUIElementBuilder& setOnClick(std::function<void(GUIButton*)> onClick);
        GUIButton* buildButton();

        // GUIText specific
        GUIElementBuilder& setText(std::wstring text);
        GUIElementBuilder& setFont(text::Font* font);
        GUIElementBuilder& setAutoScaleText(bool autoScaleText);
        GUIElementBuilder& setTextScale(float textScale);
        GUIElementBuilder& setPadding(int padding);
        GUIText* buildText();

        // GUIEditText specific
        GUIEditText* buildEditText();
    
    // TODO: Kind of a minefield if handler isn't set
    private:
        GUIHandler* handler = nullptr;
        int xPos = 0, yPos = 0;
        int width = 10, height = 10;
        bool isMovable = true, isResizable = true, isVisible = true, takesInput = true;
        int borderWidth = 10;
        int cornerRadius = 5;
        glm::vec4 color = colorMap.at("DARK GRAY");
        std::function<void(GUIButton*)> onClick = [](GUIButton*){};
        std::wstring text = L"";
        text::Font* font = text::Font::getDefaultFont();
        bool autoScaleText = true;
        float textScale = 1.0f;
        int padding = 5;
    };
}