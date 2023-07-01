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
    class GUIElement;
    class GUIElementFactory;

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
        JustManipulated,
        BeingDragged,
        BeingResized
    };

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
        bool handleInputAllElements(const SDL_Event* event, InputState* inputState);

    private:
        float windowWidth;
        float windowHeight;

        std::unordered_set<GUIElement*> rootElements;
    };

    // TODO: Switch to hierarchical input handling system instead 
    // of flat vector of all GUIElements to handle input for
    class GUIElement 
    {
        friend class GUIElementFactory;
    public:
        virtual ~GUIElement();

        void addChild(GUIElement* child);
        void removeChild(GUIElement* child);

        // TODO: Render and handleInput should be recursive on children
        // with regard for bool flags, e.g. isVisible, takesInput...
        virtual bool render() const;
        virtual bool renderChildren() const;
        virtual bool handleInput(const SDL_Event* event, InputState* inputState);
        virtual bool handleInputChildren(const SDL_Event* event, InputState* inputState);

        bool isOnElement(int x, int y);
        bool isOnCorner(int cornerNbr, int x, int y);
        void findPossibleNewCorner(int cornerNbr, int* newX, int* newY);
        bool isOnAnyCorner(int x, int y, int* cornerNbrBeingResized);

        bool getIsMovable();
        bool getIsVisible();
        bool getTakesInput();
        ElementManipulationState getManipulationState();

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

        void resize(const SDL_Event* event, InputState* inputState);
        void resizeChildren(int xOffset, int yOffset);
        void move(const SDL_Event* event, InputState* inputState);
        void offsetChildren(int xOffset, int yOffset);

        GUIHandler* handler;
        std::unordered_set<GUIElement*> children;

        int xPos, yPos;
        int width, height;
        int borderWidth;
        glm::vec4 color;
        bool isMovable, isResizable, isVisible, takesInput;
        ElementManipulationState manipulationState = ElementManipulationState::None;

        int cornerNbrBeingResized = 0;
        int accumUnderMinSizeX = 0, accumUnderMinSizeY = 0;

        GLint modelLoc, viewLoc, projectionLoc, useTextureLoc, colorLoc, textLoc, textColorLoc;
        GLuint shaderProgram, VAO, VBO, EBO;
    };

    class GUIButton : public GUIElement 
    {
        friend class GUIElementFactory;
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
        friend class GUIElementFactory;
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
        friend class GUIElementFactory;
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

        bool beingEdited;
        mutable bool lastCharacterVisible;
        mutable std::chrono::steady_clock::time_point lastBlinkTime;
        std::chrono::milliseconds blinkDuration = std::chrono::milliseconds(500);
    };

    class GUIElementFactory {
    public:
        static GUIElement* createGUIElement(GUIHandler* handler, int xPos, int yPos, int width, int height, bool isMovable = true, bool isResizable = true, bool isVisible = true, bool takesInput = true, int borderWidth = 10, glm::vec4 color = colorMap.at("DARK GRAY"));

        static GUIButton* createGUIButton(GUIHandler* handler, int xPos, int yPos, int width, int height, bool isMovable = true, bool isResizable = true, bool isVisible = true, bool takesInput = true, int borderWidth = 10, glm::vec4 color = colorMap.at("DARK GRAY"),
            std::function<void(GUIButton*)> onClick = [](GUIButton*){});

        static GUIText* createGUIText(GUIHandler* handler, int xPos, int yPos, int width, int height, bool isMovable = true, bool isResizable = true, bool isVisible = true, bool takesInput = true, int borderWidth = 10, glm::vec4 color = colorMap.at("WHITE"),
            std::wstring text = L"", text::Font* font = text::Font::getDefaultFont(), bool autoScaleText = true, float textScale = 1.0f, int padding = 0);

        static GUIEditText* createGUIEditText(GUIHandler* handler, int xPos, int yPos, int width, int height, bool isMovable = true, bool isResizable = true, bool isVisible = true, int borderWidth = 10, glm::vec4 color = colorMap.at("WHITE"),
            std::wstring text = L"", text::Font* font = text::Font::getDefaultFont(), bool autoScaleText = true, float textScale = 1.0f, int padding = 0);
    };
}