#pragma once

#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <functional>   
#include <string>

#include "input_state.h"
#include "text.h"

namespace gui
{
    class GUIElement;
    class GUIElementFactory;

    const std::unordered_map<std::string, glm::vec4> colorMap = {
        {"RED",     glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)},
        {"GREEN",   glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)},
        {"BLUE",    glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)},
        {"YELLOW",  glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)},
        {"CYAN",    glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)},
        {"MAGENTA", glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)},
        {"WHITE",   glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)},
        {"BLACK",   glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)}
    };

    class GUIHandler 
    {
    public:
        GUIHandler(float windowWidth, float windowHeight);
        ~GUIHandler();

        float getWindowHeight() const;
        float getWindowWidth() const;

        // Do not access removeElement externally, only indirectly through "delete element"
        void addElement(GUIElement* element);
        void removeElement(GUIElement* element);

        bool renderGUIElement(GUIElement* element) const;
        bool handleGUIElementInput(GUIElement* element, const SDL_Event* event, InputState* inputState);

        void addToRenderVector(GUIElement* element);
        void addToHandleInputVector(GUIElement* element);

        void prepareGUIRendering() const;
        void finishGUIRendering() const;

        bool renderWholeVector() const;
        bool handleInputWholeVector(const SDL_Event* event, InputState* inputState);

    private:
        float windowWidth;
        float windowHeight;

        std::unordered_set<GUIElement*> elements;

        std::vector<GUIElement*> renderVector;
        std::vector<GUIElement*> handleInputVector;
    };

    class GUIElement 
    {
        friend class GUIElementFactory;
    public:
        virtual ~GUIElement();

        void addChild(GUIElement* child);
        void removeChild(GUIElement* child);

        virtual bool render() const;
        virtual bool handleInput(const SDL_Event* event, InputState* inputState);

        bool isOnElement(int x, int y);

        bool getIsMovable();
        bool getIsVisible();
        bool getTakesInput();

    private:
        // Override to return respective shader for each GUIElement subtype
        virtual const char* getVertexShader(); 
        virtual const char* getFragmentShader();

        virtual bool initializeShaders();
        virtual bool initializeBuffers();
    
    protected:
        GUIElement(GUIHandler* handler, int xPos, int yPos, int width, int height, glm::vec4 color, bool isMovable = true, bool isVisible = true, bool takesInput = true);

        void move(const SDL_Event* event, InputState* inputState);
        void offsetChildren(int xOffset, int yOffset);

        GUIHandler* handler;
        std::vector<GUIElement*> children;

        int xPos, yPos;
        int width, height;
        glm::vec4 color;
        bool isMovable, isVisible, takesInput;
        bool isBeingDragged = false;

        GLint modelLoc, viewLoc, projectionLoc, useTextureLoc, colorLoc, textLoc, textColorLoc;
        GLuint shaderProgram, VAO, VBO, EBO;
    };

    class GUIButton : public GUIElement 
    {
        friend class GUIElementFactory;
    public:
        ~GUIButton() override;

        bool handleInput(const SDL_Event* event, InputState* inputState) override;

        // These are functions that can be passed to the constructor
        static void randomColor(GUIButton* button);
        static void quitApplication(GUIButton* button);

    protected:
        GUIButton(GUIHandler* handler, int xPos, int yPos, int width, int height, glm::vec4 color,
            std::function<void(GUIButton*)> onClick = [](GUIButton*){}, bool isMovable = true, bool isVisible = true, bool takesInput = true);

        std::function<void(GUIButton*)> onClick;
    };

    class GUIText : public GUIElement
    {
        friend class GUIElementFactory;
    public:
        ~GUIText() override;

        bool render() const override;
    
    protected:
        GUIText(GUIHandler* handler, int xPos, int yPos, int width, int height, glm::vec4 color,
            std::wstring text, text::Font* font, bool autoScaleText = true, float textScale = 1.0f, bool isMovable = true, bool isVisible = true, bool takesInput = true);

        const char* getVertexShader() override; 
        const char* getFragmentShader() override;

        bool initializeShaders() override;
        bool initializeBuffers() override;

        void cleanupBuffers();

        std::wstring text;
        text::Font* font;
        float textScale;

        std::vector<text::Character*> characters;
        std::unordered_map<text::Font*, std::vector<GLuint>> fontTextures;
        float baseline;
        float totalHeight, totalWidth;
        std::vector<text::Line*> lines;

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
        GUIEditText(GUIHandler* handler, int xPos, int yPos, int width, int height, glm::vec4 color,
            std::wstring text, text::Font* font, bool autoScaleText = true, float textScale = 1.0f, bool isMovable = true, bool isVisible = true);

    private:
        bool isOnText(int x, int y);
        bool isOnLine(text::Line* line, int x, int y);
        bool isOnCharacterInLine(text::Character* ch, text::Line* line, int x, int y);

        bool handleTextInput(const SDL_Keycode key);

        bool beingEdited;
    };

    class GUIElementFactory {
    public:
        static GUIElement* createGUIElement(GUIHandler* handler, int xPos, int yPos, int width, int height, glm::vec4 color, bool isMovable = true, bool isVisible = true, bool takesInput = true);

        static GUIButton* createGUIButton(GUIHandler* handler, int xPos, int yPos, int width, int height, glm::vec4 color, std::function<void(GUIButton*)> onClick = [](GUIButton*){}, bool isMovable = true, bool isVisible = true, bool takesInput = true);

        static GUIText* createGUIText(GUIHandler* handler, int xPos, int yPos, int width, int height, glm::vec4 color, std::wstring text, text::Font* font, bool autoScaleText = true, float textScale = 1.0f, bool isMovable = true, bool isVisible = true, bool takesInput = true);

        static GUIEditText* createGUIEditText(GUIHandler* handler, int xPos, int yPos, int width, int height, glm::vec4 color, std::wstring text, text::Font* font, bool autoScaleText = true, float textScale = 1.0f, bool isMovable = true, bool isVisible = true);
    };
}