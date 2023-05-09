#pragma once

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <memory>

namespace ui 
{
    // Forward declarations
    class UIRenderer;
    class UIElement;

    class UIManager
    {
    public:
        UIManager(std::shared_ptr<UIRenderer> renderer);
        ~UIManager();

        void addElement(std::shared_ptr<UIElement> element);
        void handleInput(const SDL_Event& event);
        void render();
    private:
        std::shared_ptr<UIRenderer> renderer;
        std::vector<std::shared_ptr<UIElement>> elements;
    };
    
    class UIRenderer
    {
    public:
        UIRenderer();
        ~UIRenderer();

        void render(UIElement& element);
    private:
        const GLchar *vertexShaderSource, *fragmentShaderSource;
        GLuint shaderProgram;
    };
    
    class UIElement // Abstract class
    {
    public:
        UIElement(int x, int y, int width, int height);
        virtual ~UIElement();

        virtual void handleInput(const SDL_Event& event) = 0;
        virtual void render(UIRenderer& renderer) = 0;
    protected:
        friend class UIRenderer;

        int x, y, width, height;
    };

    class UIButton : public UIElement
    {
    public:
        UIButton(int x, int y, int width, int height) : UIElement(x, y, width, height) {}
        ~UIButton() override;

        void handleInput(const SDL_Event& event) override;
        void handleClick(int mouseX, int mouseY);
        void render(UIRenderer& renderer) override;
    };
}