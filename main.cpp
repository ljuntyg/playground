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

    SDL_DisplayMode DM;
    if (SDL_GetDesktopDisplayMode(0, &DM) != 0)
    {
        std::cerr << "Failed to get display mode: " << SDL_GetError() << std::endl;
        return 1;
    }

    float windowWidth = DM.w / 2;
    float windowHeight = DM.h / 2;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24); // Request a 24-bit depth buffer

    SDL_Window *window = SDL_CreateWindow("Playground - FPS: ", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN); // Fix fixed resolution
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

    auto RENDERER = std::make_shared<renderer::Renderer>(windowWidth, windowHeight); // Renderer to be used throughout program

    auto uiRenderer = std::make_shared<ui::UIRenderer>(RENDERER);
    auto uiManager = std::make_shared<ui::UIManager>(uiRenderer);

    auto uiWindow1 = std::make_shared<ui::UIButton>(10, 10, 75, 75, RENDERER->colorMap.at("GREEN"), uiManager);
    uiManager->addElement(uiWindow1);

    auto uiBox1 = std::make_shared<ui::UIBox>(5, 5, 75, 75, RENDERER->colorMap.at("RED"), uiManager);
    uiWindow1->addChild(uiBox1);

    auto uiBox2 = std::make_shared<ui::UIBox>(5, 5, 75, 75, RENDERER->colorMap.at("BLUE"), uiManager);
    uiBox1->addChild(uiBox2);

    auto textManager = std::make_shared<text::TextManager>();
    auto text1 = std::make_shared<text::Text>(L"PLAYGROUND!!", 0.25, textManager);
    auto uiText1 = std::make_shared<ui::UIText>(text1, 95, 65, 1, 1, RENDERER->colorMap.at("WHITE"), uiManager);

    auto text2 = std::make_shared<text::Text>(L"", 0.15, textManager);
    auto uiText2 = std::make_shared<ui::UIText>(text2, 15, RENDERER->WINDOW_HEIGHT - 10, 1, 1, RENDERER->colorMap.at("WHITE"), uiManager);

    auto text3 = std::make_shared<text::Text>(L"Change\nScene", 0.1, textManager);
    auto uiText3 = std::make_shared<ui::UIText>(text3, 5, 60, 1, 1, RENDERER->colorMap.at("WHITE"), uiManager);

    uiManager->addElement(uiText1);
    uiManager->addElement(uiText2);
    uiBox2->addChild(uiText3);

    Uint32 frameCounter = 0;
    Uint32 timerFPS = SDL_GetTicks();

    bool firstMouseMotion = true;
    bool quit = false;
    SDL_Event event;

    while (!quit)
    {
        text2->text = std::to_wstring(RENDERER->cameraPos.x) + L"\n" + std::to_wstring(RENDERER->cameraPos.y) + L"\n" + std::to_wstring(RENDERER->cameraPos.z);
        text2->calculateVertices();

        if (RENDERER->RENDERER_STATE == renderer::RENDERER_PAUSE)
        {
            std::cout << "paused" << std::endl;
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

                    dx += mouseX * RENDERER->mouseSensitivity;
                    dy -= mouseY * RENDERER->mouseSensitivity;

                    // Call function to update camera based on mouse motion
                    RENDERER->onYawPitch(dx, dy);
                }
            }

            uiManager->handleInput(event);
        }

        if (firstMouseMotion) { // When updateCamera is called first time the lookDir will change, call it pre-emptively to avoid visible snap
            firstMouseMotion = false;
            RENDERER->onYawPitch(0, 0);
        }

        const Uint8 *keyboardState = SDL_GetKeyboardState(NULL);

        // Update camera and target position based on key input
        if (keyboardState[SDL_SCANCODE_W])
        {
            RENDERER->onKeys(0);
        }
        if (keyboardState[SDL_SCANCODE_S])
        {
            RENDERER->onKeys(1);
        }
        if (keyboardState[SDL_SCANCODE_A])
        {
            RENDERER->onKeys(2);
        }
        if (keyboardState[SDL_SCANCODE_D])
        {
            RENDERER->onKeys(3);
        }
        if (keyboardState[SDL_SCANCODE_LSHIFT])
        {
            RENDERER->onKeys(4);
        }
        if (keyboardState[SDL_SCANCODE_LCTRL])
        {
            RENDERER->onKeys(5);
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Create the model, view, and projection matrices
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        glm::mat4 viewMatrix = glm::lookAt(
            RENDERER->cameraPos, // Camera position
            RENDERER->targetPos, // Target position
            RENDERER->cameraUp  // Up vector
        );
        glm::mat4 projectionMatrix = glm::perspective(
            RENDERER->FOV, // Field of view
            RENDERER->WINDOW_WIDTH / RENDERER->WINDOW_HEIGHT, // Aspect ratio
            RENDERER->NEAR_DIST, // Near plane
            RENDERER->FAR_DIST // Far plane
        );

        // Render OpenGL content here
        RENDERER->drawObject(RENDERER->targetObj, modelMatrix, viewMatrix, projectionMatrix);

        // Draw all elements in ui manager
        glEnable(GL_BLEND); // Or else the transparent parts of the texture will be rendered white
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);
        uiManager->render();
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

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