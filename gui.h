#pragma once

#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <queue>
#include <unordered_set>

namespace gui
{
    class GUIElement;

    class GUIHandler 
    {
    public:
        GUIHandler();
        ~GUIHandler();

        bool renderGUIElement(GUIElement* element) const;
        bool handleGUIElementInput(GUIElement* element, const SDL_Event* event);

        void addToRenderQueue(GUIElement* element);
        void addToHandleInputQueue(GUIElement* element);

        bool renderWholeQueue();
        bool handleInputWholeQueue(const SDL_Event* event);

    private:
        friend class GUIElement;

        // These methods should only be accessed automatically on creation/deletion of elements using handler
        void addElement(GUIElement* element);
        void removeElement(GUIElement* element);

        std::unordered_set<GUIElement*> elements;

        std::queue<GUIElement*> renderQueue;
        std::queue<GUIElement*> handleInputQueue;
    };


    class GUIElement 
    {
    public:
        GUIElement(GUIHandler* handler, int xPos, int yPos, int width, int height, bool isMovable = true, bool isVisible = true, bool takesInput = true);
        virtual ~GUIElement();

        // Override to return respective shader for each GUIElement subtype
        virtual const char* getGuiVertexShader(); 
        virtual const char* getGuiFragmentShader();
        bool initializeShaders();

        virtual bool render() const;
        virtual bool handleInput(const SDL_Event* event);

        void addChild(GUIElement* child);
        void removeChild(GUIElement* child);

    private:
        friend class GUIHandler;

        GUIHandler* handler;
        std::vector<GUIElement*> children;
        int xPos, yPos;
        int width, height;
        bool isMovable, isVisible, takesInput;

        GLint modelLoc, viewLoc, projectionLoc, useTextureLoc, objectColorLoc;
        GLuint shaderProgram, VAO, VBO, EBO;
    };

    class GUIButton : public GUIElement 
    {
    public:
        GUIButton();
        ~GUIButton() override;

        bool render() const override;
        bool handleInput(const SDL_Event* event) override;
    };
}