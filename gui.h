#pragma once

#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <functional>   
#include <string>

#include "mouse_state.h"
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
        bool handleGUIElementInput(GUIElement* element, const SDL_Event* event, MouseState* mouseState);

        void addToRenderVector(GUIElement* element);
        void addToHandleInputVector(GUIElement* element);

        bool renderWholeVector();
        bool handleInputWholeVector(const SDL_Event* event, MouseState* mouseState);

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
        virtual bool handleInput(const SDL_Event* event, MouseState* mouseState);

        bool getIsMovable();
        bool getIsVisible();
        bool getTakesInput();

    private:
        // Override to return respective shader for each GUIElement subtype
        virtual const char* getVertexShader(); 
        virtual const char* getFragmentShader();

        bool initializeShaders();
        bool initializeBuffers();
    
    protected:
        GUIElement(GUIHandler* handler, int xPos, int yPos, int width, int height, glm::vec4 color, bool isMovable = true, bool isVisible = true, bool takesInput = true);

        void move(const SDL_Event* event, MouseState* mouseState);
        void offsetChildren(int xOffset, int yOffset);

        GUIHandler* handler;
        std::vector<GUIElement*> children;

        int xPos, yPos;
        int width, height;
        glm::vec4 color;
        bool isMovable, isVisible, takesInput;
        bool isBeingDragged = false;

        GLint modelLoc, viewLoc, projectionLoc, useTextureLoc, colorLoc, textLoc;
        GLuint shaderProgram, VAO, VBO, EBO;
    };

    class GUIButton : public GUIElement 
    {
        friend class GUIElementFactory;
    public:
        ~GUIButton() override;

        bool handleInput(const SDL_Event* event, MouseState* mouseState) override;

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
        bool handleInput(const SDL_Event* event, MouseState* mouseState) override;
    
    protected:
        GUIText(GUIHandler* handler, int xPos, int yPos, int width, int height, glm::vec4 color,
            std::vector<text::Character*>* text, bool isMovable = true, bool isVisible = true, bool takesInput = true);

        const char* getVertexShader() override; 
        const char* getFragmentShader() override;

        bool generateVertices();
        bool loadFontTextures();

        std::vector<text::Character*>* text;
        std::unordered_map<text::Font*, std::vector<GLuint>> fontTextures;
    };

    class GUIElementFactory {
    public:
        static GUIElement* createGUIElement(GUIHandler* handler, int xPos, int yPos, int width, int height, glm::vec4 color, bool isMovable = true, bool isVisible = true, bool takesInput = true);

        static GUIButton* createGUIButton(GUIHandler* handler, int xPos, int yPos, int width, int height, glm::vec4 color, std::function<void(GUIButton*)> onClick = [](GUIButton*){}, bool isMovable = true, bool isVisible = true, bool takesInput = true);

        static GUIText* createGUIText(GUIHandler* handler, int xPos, int yPos, int width, int height, glm::vec4 color, std::vector<text::Character*>* text, bool isMovable = true, bool isVisible = true, bool takesInput = true);
    };
}