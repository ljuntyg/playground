#include "ui.h"

namespace ui
{
    UIWindow::~UIWindow() {}

    void UIWindow::handleInput(const SDL_Event& event)
    {

    }

    void UIWindow::render(UIRenderer& renderer)
    {
        renderer.render(*this);
    }
}