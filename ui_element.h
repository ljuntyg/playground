#pragma once

#include <SDL2/SDL.h>

#include "ui_renderer.h"

class UIElement {
public:
    UIElement(int x, int y, int width, int height);
    virtual ~UIElement() {}

    virtual void render(UIRenderer& renderer) = 0;
    virtual void update(float deltaTime) = 0;
    virtual void handleInput(const SDL_Event& event) = 0;

protected:
    int x, y, width, height;
};