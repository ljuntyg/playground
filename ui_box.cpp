#include <iostream>

#include "ui.h"
#include "renderer.h"

namespace ui
{
    UIBox::~UIBox() {}

    void UIBox::handleInput(const SDL_Event& event) {}

    void UIBox::render(UIRenderer& renderer)
    {
        renderer.render(*this);
    }
}