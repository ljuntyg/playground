#pragma once

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <memory>

#include "text.h"
#include "renderer.h"

namespace ui 
{
    // Forward declarations
    class UIRenderer;
    class UIElement;
    class UIText;

    class UIManager
    {
    public:
        UIManager(std::shared_ptr<UIRenderer> uiRenderer);
        ~UIManager();

        void addElement(std::shared_ptr<UIElement> element);
        void handleInput(const SDL_Event& event);
        void render();

        std::shared_ptr<UIRenderer> uiRenderer;
    private:
        std::vector<std::shared_ptr<UIElement>> elements;
    };
    
    class UIRenderer
    {
    public:
        std::shared_ptr<renderer::Renderer> RENDERER;

        UIRenderer(std::shared_ptr<renderer::Renderer> RENDERER);
        ~UIRenderer();

        void render(UIElement& element);
        void render(UIText& textElement); // For handling text rendering specifically
    private:
        const GLchar *UIVertexShaderSource = R"glsl(
            #version 330 core
            layout (location = 0) in vec3 aPos;

            uniform mat4 model;
            uniform mat4 projection;

            void main()
            {
                gl_Position = projection * model * vec4(aPos, 1.0);
            }
        )glsl";
        const GLchar *UIFragmentShaderSource = R"(
            #version 330 core
            out vec4 FragColor;

            uniform vec4 color;

            void main()
            {
                FragColor = color;
            }
        )";
        const GLchar *textVertexShaderSource = R"glsl(
            #version 330 core
            layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
            out vec2 TexCoords;

            uniform mat4 model;
            uniform mat4 projection;

            void main()
            {
                vec4 pos = model * vec4(vertex.xy, 0.0, 1.0);
                gl_Position = projection * pos;
                TexCoords = vertex.zw;
            }  
        )glsl";
        const GLchar *textFragmentShaderSource = R"glsl(
            #version 330 core
            in vec2 TexCoords;
            out vec4 color;

            uniform sampler2D text;
            uniform vec4 textColor;

            void main()
            {    
                vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
                color = textColor * sampled;
            }   
        )glsl";
        GLuint shaderProgram; // TODO: dont initialize in render method
    };
    
    class UIElement : public std::enable_shared_from_this<UIElement> // Abstract class, enable_shared_from_this required for when adding self as parent to child when addChild called
    {
    public:
        UIElement(int x, int y, int width, int height, const glm::vec4& color, std::shared_ptr<UIManager> uiManager);
        virtual ~UIElement();

        virtual void handleInput(const SDL_Event& event) = 0;
        virtual void render(UIRenderer& uiRenderer) = 0;

        void addChild(std::shared_ptr<UIElement> element);
    protected:
        friend class UIRenderer;
        std::shared_ptr<UIManager> uiManager;

        int x, y, width, height;
        glm::vec4 color;

        GLuint VAO, VBO, EBO;

        std::shared_ptr<UIElement> parent;
        std::vector<std::shared_ptr<UIElement>> children;
    };

    class UIBox : public UIElement
    {
    public:
        UIBox(int x, int y, int width, int height, const glm::vec4& color, std::shared_ptr<UIManager> uiManager)
            : UIElement(x, y, width, height, color, uiManager) {}
        ~UIBox() override;

        void handleInput(const SDL_Event& event) override;
        void render(UIRenderer& uiRenderer) override;
    };

    class UIButton : public UIElement
    {
    public:
        UIButton(int x, int y, int width, int height, const glm::vec4& color, std::shared_ptr<UIManager> uiManager)
            : UIElement(x, y, width, height, color, uiManager) {}
        ~UIButton() override;

        void handleInput(const SDL_Event& event) override;
        void render(UIRenderer& uiRenderer) override;

        void handleClick(int mouseX, int mouseY);
    };

    class UIText : public UIElement
    {
    public:
        UIText(std::shared_ptr<text::Text> text, int x, int y, int width, int height, const glm::vec4& color, std::shared_ptr<UIManager> uiManager);
        ~UIText() override;

        void handleInput(const SDL_Event& event) override;
        void render(UIRenderer& uiRenderer) override; // Use overloaded method in UIRenderer made for UIText
    private:
        friend class UIRenderer;

        std::shared_ptr<text::Text> text;
    };
}