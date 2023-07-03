#pragma once

#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <functional>   
#include <string>
#include <chrono>

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

    // TODO: Since GUIElement lifetime is bound to GUIHandler,
    // ensure that with the new hierarchical structure, root GUIElement's 
    // children are recursively deleted when a GUIHandler is deleted
    class GUIHandler : public Subscriber, public Publisher
    {
    public:
        GUIHandler(float windowWidth, float windowHeight);
        ~GUIHandler();

        void notify(const event::Event* event) override;

        float getWindowHeight() const;
        float getWindowWidth() const;

        // Do not access removeElement externally, only indirectly through "delete element"
        void addElement(GUIElement* element);
        void removeElement(GUIElement* element);

        void prepareGUIRendering() const;
        void finishGUIRendering() const;

        bool renderAllElements() const;
        bool receiveInputAllElements(const SDL_Event* event, InputState* inputState);

        std::unordered_set<GUIElement*> getRootElements();

        bool isOrContainsActiveElement(GUIElement* element);
        GUIElement* getActiveElement();
        void setActiveElement(GUIElement* element);

    private:
        float windowWidth;
        float windowHeight;

        std::unordered_set<GUIElement*> rootElements;
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
        virtual bool handleInput(const SDL_Event* event, InputState* inputState);
        bool receiveInput(const SDL_Event* event, InputState* inputState);
        bool receiveInputChildren(const SDL_Event* event, InputState* inputState);

        bool isOnElement(int x, int y);
        bool isOnCorner(int cornerNbr, int x, int y);
        void findPossibleNewCorner(int cornerNbr, int* newX, int* newY);
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
        GUIElement(GUIHandler* handler, int xPos, int yPos, int width, int height, bool isMovable = true, bool isResizable = true, bool isVisible = true, bool takesInput = true, int borderWidth = 10, glm::vec4 color = colorMap.at("DARK GRAY"));

        // Override for GUIText text regeneration after resize
        virtual void onResize();

        // Resize and move return true if the event was consumed, false otherwise
        bool resize(const SDL_Event* event, InputState* inputState);
        void resizeChildren(int xOffset, int yOffset);
        bool move(const SDL_Event* event, InputState* inputState);
        void offsetChildren(int xOffset, int yOffset);

        GUIHandler* handler;
        std::unordered_set<GUIElement*> children;

        int xPos, yPos;
        int width, height;
        int borderWidth;
        glm::vec4 color;
        bool isMovable, isResizable, isVisible, takesInput;
        ElementManipulationState manipulationStateResize = ElementManipulationState::None;
        ElementManipulationState manipulationStateMove = ElementManipulationState::None;

        int cornerNbrBeingResized = 0;
        int accumUnderMinSizeX = 0, accumUnderMinSizeY = 0;

        GLint modelLoc, viewLoc, projectionLoc, useTextureLoc, colorLoc, textLoc, textColorLoc;
        GLuint shaderProgram, VAO, VBO, EBO;
    };

    class GUIButton : public GUIElement 
    {
        friend class GUIElementBuilder;
    public:
        ~GUIButton() override;

        bool handleInput(const SDL_Event* event, InputState* inputState) override;

        // These are functions that can be passed to the constructor (onClick)
        static void randomColor(GUIButton* button);
        static void quitApplication(GUIButton* button);

    protected:
        GUIButton(GUIHandler* handler, int xPos, int yPos, int width, int height, bool isMovable = true, bool isResizable = true, bool isVisible = true, bool takesInput = true, int borderWidth = 10, glm::vec4 color = colorMap.at("DARK GRAY"),
            std::function<void(GUIButton*)> onClick = [](GUIButton*){});

        std::function<void(GUIButton*)> onClick;
    };

    class GUIText : public GUIElement
    {
        friend class GUIElementBuilder;
    public:
        ~GUIText() override;

        bool render() const override;

    protected:
        GUIText(GUIHandler* handler, int xPos, int yPos, int width, int height, bool isMovable = true, bool isResizable = true, bool isVisible = true, bool takesInput = true, int borderWidth = 10, glm::vec4 color = colorMap.at("WHITE"),
            std::wstring text = L"", text::Font* font = text::Font::getDefaultFont(), bool autoScaleText = true, float textScale = 1.0f, int padding = 0);

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
        std::vector<GLuint> characterVAOs;
        GLint projectionLoc, modelLoc, textLoc, textColorLoc;
    
    private:
        bool loadFontTextures();
        std::vector<float> calculateVertices(text::Character* ch, float x, float y);
    };

    class GUIEditText : public GUIText
    {
        friend class GUIElementBuilder;
    public:
        ~GUIEditText() override;

        bool handleInput(const SDL_Event* event, InputState* inputState) override;

    protected:
        GUIEditText(GUIHandler* handler, int xPos, int yPos, int width, int height, bool isMovable = true, bool isResizable = true, bool isVisible = true, int borderWidth = 10, glm::vec4 color = colorMap.at("WHITE"),
            std::wstring text = L"", text::Font* font = text::Font::getDefaultFont(), bool autoScaleText = true, float textScale = 1.0f, int padding = 0);

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
        GUIElementBuilder& setFlags(bool isMovable = true, bool isResizable = true, bool isVisible = true, bool takesInput = true);
        GUIElementBuilder& setBorderWidth(int borderWidth);
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
        glm::vec4 color = colorMap.at("DARK GRAY");
        std::function<void(GUIButton*)> onClick = [](GUIButton*){};
        std::wstring text = L"";
        text::Font* font = text::Font::getDefaultFont();
        bool autoScaleText = true;
        float textScale = 1.0f;
        int padding = 0;
    };
}