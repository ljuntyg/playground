#include <iostream>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "renderer.h"

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window *window = SDL_CreateWindow("SDL2 OpenGL Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, renderer::WINDOW_WIDTH, renderer::WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (!window)
    {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!context)
    {
        std::cerr << "Failed to create OpenGL context: " << SDL_GetError() << std::endl;
        return 1;
    }

    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(glewError) << std::endl;
        return 1;
    }

    bool firstMouseMotion = true;
    bool quit = false;
    SDL_Event event;

    while (!quit)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = true;
            } 
            else if (event.type == SDL_MOUSEMOTION) 
            {
                // Check if the left mouse button is pressed
                if (event.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT)) 
                {
                    int mouseX = event.motion.xrel;
                    int mouseY = event.motion.yrel;

                    float dx = 0.0f;
                    float dy = 0.0f;

                    dx += mouseX * renderer::mouseSensitivity;
                    dy -= mouseY * renderer::mouseSensitivity;

                    // Call function to update camera based on mouse motion
                    renderer::updateCamera(dx, dy);
                }
            }
        }

        if (firstMouseMotion) {
            firstMouseMotion = false;
            renderer::updateCamera(0, 0);
        }

        const Uint8 *keyboardState = SDL_GetKeyboardState(NULL);

        glm::vec3 front = renderer::lookDir;
        glm::vec3 right = glm::normalize(glm::cross(renderer::cameraUp, front));
        glm::vec3 up = renderer::cameraUp;

        // Update camera and target position based on key input
        if (keyboardState[SDL_SCANCODE_W])
        {
            front *= renderer::cameraSpeed;
            renderer::cameraPos += front;
            renderer::targetPos += front;
        }
        if (keyboardState[SDL_SCANCODE_S])
        {
            front *= renderer::cameraSpeed;
            renderer::cameraPos -= front;
            renderer::targetPos -= front;
        }
        if (keyboardState[SDL_SCANCODE_A])
        {
            right *= renderer::cameraSpeed;
            renderer::cameraPos += right;
            renderer::targetPos += right;
        }
        if (keyboardState[SDL_SCANCODE_D])
        {
            right *= renderer::cameraSpeed;
            renderer::cameraPos -= right;
            renderer::targetPos -= right;
        }
        if (keyboardState[SDL_SCANCODE_LSHIFT])
        {
            up *= renderer::cameraSpeed;
            renderer::cameraPos += up;
            renderer::targetPos += up;
        }
        if (keyboardState[SDL_SCANCODE_LCTRL])
        {
            up *= renderer::cameraSpeed;
            renderer::cameraPos -= up;
            renderer::targetPos -= up;
        }

        /* std::cout << "Camera Position: (" << renderer::cameraPos.x << ", " << renderer::cameraPos.y << ", " << renderer::cameraPos.z << ")\n";
        std::cout << "Front Vector: (" << front.x << ", " << front.y << ", " << front.z << ")\n"; */

        glClearColor(0.0f, 0.0f, 00.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Create the model, view, and projection matrices
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        glm::mat4 viewMatrix = glm::lookAt(
            renderer::cameraPos, // Camera position
            renderer::targetPos, // Target position
            renderer::cameraUp  // Up vector
        );
        glm::mat4 projectionMatrix = glm::perspective(
            renderer::FOV, // Field of view
            renderer::WINDOW_WIDTH / renderer::WINDOW_HEIGHT, // Aspect ratio
            renderer::NEAR_DIST, // Near plane
            renderer::FAR_DIST // Far plane
        );

        std::vector<Mesh> object = {renderer::cube};

        // Render your OpenGL content here
        renderer::drawObject(renderer::targetObj, modelMatrix, viewMatrix, projectionMatrix);

        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}