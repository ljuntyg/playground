#include <iostream>

#include "renderer.h"

int main(int argc, char* args[])
{
    renderer::Renderer* renderer = new renderer::Renderer();
    if (renderer->RENDERER_STATE == renderer::RENDERER_CREATE_ERROR)
    {
        return -1;
    }

    delete renderer;

    return 0;
}
