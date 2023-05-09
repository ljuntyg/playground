#include <iostream>

#include "ui.h"
#include "renderer.h"

namespace ui
{
    UIButton::~UIButton() {}

    void UIButton::handleInput(const SDL_Event& event)
    {
        if (event.type == SDL_MOUSEBUTTONDOWN)
        {
            // Get the mouse click coordinates
            int mouseX = event.button.x;
            int mouseY = event.button.y;

            // SDL coordinates are centered at top left with Y pointing down, OpenGL is centered at bottom left with Y up, adjust Y
            mouseY = renderer::WINDOW_HEIGHT - mouseY;

            // Check if the click is within the range of the UI window
            if (mouseX >= x && mouseX <= x + width && mouseY >= y && mouseY <= y + height)
            {
                // Handle the mouse click event here
                handleClick(mouseX, mouseY);
            }
        }
    }

    void UIButton::handleClick(int mouseX, int mouseY)
    {
        std::cout << "test" << std::endl;
        renderer::nextTargetObj();
    }

    void UIButton::render(UIRenderer& renderer)
    {
        renderer.render(*this);
    }
}