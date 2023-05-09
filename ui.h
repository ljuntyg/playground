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
    
    class UIElement : public std::enable_shared_from_this<UIElement> // Abstract class, enable_shared_from_this required for when adding self as parent to child when addChild called
    {
    public:
        UIElement(int x, int y, int width, int height, const glm::vec4& color);
        virtual ~UIElement();

        virtual void handleInput(const SDL_Event& event) = 0;
        virtual void render(UIRenderer& renderer) = 0;

        void addChild(std::shared_ptr<UIElement> element);
    protected:
        friend class UIRenderer;

        int x, y, width, height;
        glm::vec4 color;

        std::shared_ptr<UIElement> parent;
        std::vector<std::shared_ptr<UIElement>> children;
    };

    class UIBox : public UIElement
    {
    public:
        UIBox(int x, int y, int width, int height, const glm::vec4& color) : UIElement(x, y, width, height, color) {}
        ~UIBox() override;

        void handleInput(const SDL_Event& event) override;
        void render(UIRenderer& renderer) override;
    };

    class UIButton : public UIElement
    {
    public:
        UIButton(int x, int y, int width, int height, const glm::vec4& color) : UIElement(x, y, width, height, color) {}
        ~UIButton() override;

        void handleInput(const SDL_Event& event) override;
        void handleClick(int mouseX, int mouseY);
        void render(UIRenderer& renderer) override;
    };
}