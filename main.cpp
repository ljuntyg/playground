#include <iostream>
#include <sstream>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "renderer.h"
#include "ui.h"
#include "text.h"

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
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24); // Request a 24-bit depth buffer

    SDL_Window *window = SDL_CreateWindow("Playground - FPS: ", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, renderer::WINDOW_WIDTH, renderer::WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
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

    // Enable back face culling and depth buffer
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    std::shared_ptr<text::TextManager> textManager = std::make_shared<text::TextManager>();
    text::Text text1 = text::Text("testing!", textManager);

    std::shared_ptr<ui::UIRenderer> uiRenderer = std::make_shared<ui::UIRenderer>(ui::UIRenderer());
    ui::UIManager uiManager = ui::UIManager(uiRenderer);

    std::shared_ptr<ui::UIButton> uiWindow1 = std::make_shared<ui::UIButton>(10, 10, 50, 50, renderer::colorMap.at("RED"));
    uiManager.addElement(uiWindow1);

    std::shared_ptr<ui::UIBox> uiBox1 = std::make_shared<ui::UIBox>(5, 5, 20, 20, renderer::colorMap.at("BLUE"));
    uiWindow1->addChild(uiBox1);

    std::shared_ptr<ui::UIBox> uiBox2 = std::make_shared<ui::UIBox>(5, 5, 10, 10, renderer::colorMap.at("GREEN"));
    uiBox1->addChild(uiBox2);

    Uint32 frameCounter = 0;
    Uint32 timerFPS = SDL_GetTicks();

    bool firstMouseMotion = true;
    bool quit = false;
    SDL_Event event;

    while (!quit)
    {
        if (renderer::RENDERER_STATE == renderer::RENDERER_PAUSE)
        {
            continue;
        }

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
                    renderer::onYawPitch(dx, dy);
                }
            }
            
            uiManager.handleInput(event);
        }

        if (firstMouseMotion) { // When updateCamera is called first time the lookDir will change, call it pre-emptively to avoid visible snap
            firstMouseMotion = false;
            renderer::onYawPitch(0, 0);
        }

        const Uint8 *keyboardState = SDL_GetKeyboardState(NULL);

        // Update camera and target position based on key input
        if (keyboardState[SDL_SCANCODE_W])
        {
            renderer::onKeys(0);
        }
        if (keyboardState[SDL_SCANCODE_S])
        {
            renderer::onKeys(1);
        }
        if (keyboardState[SDL_SCANCODE_A])
        {
            renderer::onKeys(2);
        }
        if (keyboardState[SDL_SCANCODE_D])
        {
            renderer::onKeys(3);
        }
        if (keyboardState[SDL_SCANCODE_LSHIFT])
        {
            renderer::onKeys(4);
        }
        if (keyboardState[SDL_SCANCODE_LCTRL])
        {
            renderer::onKeys(5);
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

        // Render OpenGL content here
        renderer::drawObject(renderer::targetObj, modelMatrix, viewMatrix, projectionMatrix);

        // Draw all elements in ui manager
        glDisable(GL_DEPTH_TEST);
        uiManager.render();
        glEnable(GL_DEPTH_TEST);

        SDL_GL_SwapWindow(window);

        frameCounter++;
        Uint32 ticksNow = SDL_GetTicks();
        Uint32 elapsedTime = ticksNow - timerFPS;

        if (elapsedTime >= 1000)
        {
            std::stringstream ss;
            ss << "Playground - FPS: " << frameCounter;
            SDL_SetWindowTitle(window, ss.str().c_str());
            frameCounter = 0;
            timerFPS = SDL_GetTicks();
        }
    }

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}