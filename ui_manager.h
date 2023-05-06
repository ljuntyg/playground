#pragma once

#include <SDL2/SDL.h>

#include "ui_renderer.h"

class UIManager {
public:
    UIManager();
    ~UIManager();

    void render();
    void update(float deltaTime);
    void handleInput(const SDL_Event& event);

    void addElement(std::shared_ptr<UIElement> element);

private:
    UIRenderer renderer;
    std::vector<std::shared_ptr<UIElement>> elements;
};