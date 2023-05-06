#pragma once

class UIRenderer {
public:
    UIRenderer();
    ~UIRenderer();

    void init();
    void render(UIElement& element);

private:
    // OpenGL-related data like shaders, textures, and buffers
};