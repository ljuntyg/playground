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

        void addElement(GUIElement* element);
        void removeElement(GUIElement* element);

        bool renderGUIElement(GUIElement* element) const;
        bool handleGUIElementInput(GUIElement* element, const SDL_Event* event);

        void addToRenderQueue(GUIElement* element);
        void addToHandleInputQueue(GUIElement* element);

        bool renderWholeQueue();
        bool handleInputWholeQueue(const SDL_Event* event);

    private:
        std::unordered_set<GUIElement*> elements;

        std::queue<GUIElement*> renderQueue;
        std::queue<GUIElement*> handleInputQueue;
    };


    class GUIElement 
    {
    public:
        GUIElement(GUIHandler* handler, int xPos, int yPos, int width, int height, bool isVisible = true, bool takesInput = false);
        virtual ~GUIElement();

        virtual bool render() const = 0;
        virtual bool handleInput(const SDL_Event* event) = 0;

        void addChild(GUIElement* child);
        void removeChild(GUIElement* child);

    private:
        friend class GUIHandler;

        GUIHandler* handler;
        std::vector<GUIElement*> children;
        int xPos, yPos;
        int width, height;
        bool isVisible, takesInput;
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