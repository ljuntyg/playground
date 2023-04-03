#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>

class Renderer {
    public:
        static void drawTriangle(SDL_Renderer &renderer);
};

#endif // RENDERER_H
