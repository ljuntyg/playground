#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <algorithm>

#include "renderer.h"
#include "text.h"
#include "shaders.h"
#include "gui.h"

namespace renderer
{
    bool SDL_GLAD_init(SDL_Window** window, SDL_GLContext* context, float* WINDOW_WIDTH, float* WINDOW_HEIGHT) 
    {
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
            return false;
        }

        SDL_DisplayMode DM;
        if (SDL_GetDesktopDisplayMode(0, &DM) != 0)
        {
            std::cerr << "Failed to get display mode: " << SDL_GetError() << std::endl;
            return false;
        }

        *WINDOW_WIDTH = (float)(DM.w / 1.5);
        *WINDOW_HEIGHT = (float)(DM.h / 1.5);

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

        *window = SDL_CreateWindow("Playground", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
            (int)*WINDOW_WIDTH, (int)*WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
        if (*window == nullptr)
        {
            std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
            return false;
        }

        *context = SDL_GL_CreateContext(*window);
        if (*context == nullptr)
        {
            std::cerr << "Failed to create OpenGL context: " << SDL_GetError() << std::endl;
            return false;
        }

        if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
        {
            std::cerr << "Failed to initialize GLAD" << std::endl;
            return false;
        }

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        return true;
    }

    Renderer::Renderer()
    {
        window = nullptr;
        context = nullptr;
        if (SDL_GLAD_init(&window, &context, &WINDOW_WIDTH, &WINDOW_HEIGHT) &&
            initializeObject() &&
            initializeShaders())
        {
            RENDERER_STATE = RENDERER_CREATED;
        } 
        else 
        {
            RENDERER_STATE = RENDERER_CREATE_ERROR;
            return;
        }

        run();
    }

    Renderer::~Renderer() 
    {   
        // Clean up shaderProgram, VAO, VBO, EBO
        glDeleteProgram(shaderProgram);
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);

        // SDL clean up
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    bool Renderer::initializeObject()
    {
        allObjPaths = getObjFilePaths(OBJ_PATH);
        if (allObjPaths.size() == 0)
        {
            std::cerr << "No obj files found" << std::endl;
            return false;
        }

        targetObj = getTargetObjMeshes();
        if (targetObj.size() == 0)
        {
            std::cerr << "Object meshes failed to load" << std::endl;
            return false;
        }

        for (auto& mesh : targetObj) 
        {
            // Generate and bind a VAO for the mesh
            GLuint VAO;
            glGenVertexArrays(1, &VAO);
            glBindVertexArray(VAO);

            // Upload vertex data
            GLuint VBO;
            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, mesh.Vertices.size() * sizeof(objl::Vertex), &mesh.Vertices[0], GL_STATIC_DRAW);

            // Upload index data
            GLuint EBO;
            glGenBuffers(1, &EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.Indices.size() * sizeof(unsigned int), &mesh.Indices[0], GL_STATIC_DRAW);

            // Position attribute
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(objl::Vertex), (void*)0);
            glEnableVertexAttribArray(0);

            // Normal attribute
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(objl::Vertex), (void*)offsetof(objl::Vertex, Normal));
            glEnableVertexAttribArray(1);

            // Texture Coordinate attribute
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(objl::Vertex), (void*)offsetof(objl::Vertex, TextureCoordinate));
            glEnableVertexAttribArray(2);

            // Store the VAO in the mesh
            mesh.VAO = VAO;
        }

        return true;
    }

    bool Renderer::initializeShaders()
    {
        shaderProgram = shaders::createShaderProgram(shaders::rendererVertexShaderSource, shaders::rendererFragmentShaderSource);
        if (shaderProgram == 0)
        {
            std::cerr << "Failed to create shader program" << std::endl;
            return false;
        }

        modelLoc = glGetUniformLocation(shaderProgram, "model");
        viewLoc = glGetUniformLocation(shaderProgram, "view");
        projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        useTextureLoc = glGetUniformLocation(shaderProgram, "useTexture");
        objectColorLoc = glGetUniformLocation(shaderProgram, "objectColor");

        glUseProgram(shaderProgram);
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        return true;
    }

    void Renderer::run()
    {
        auto testFont = new text::Font("Geo", "res/fonts/Geo");
        auto testFont2 = new text::Font("Silkscreen", "res/fonts/Silkscreen");
        auto testHandler = new gui::GUIHandler(WINDOW_WIDTH, WINDOW_HEIGHT);

        auto testElement = gui::GUIElementFactory::createGUIElement(testHandler, 40, 40, 50, 50, gui::colorMap.at("BLUE"));
        auto testChild = gui::GUIElementFactory::createGUIElement(testHandler, 10, 10, 30, 30, gui::colorMap.at("RED"), false, true, false);
        auto testChild2 = gui::GUIElementFactory::createGUIElement(testHandler, 5, 5, 15, 15, gui::colorMap.at("YELLOW"), false, true, false);
        auto testButton = gui::GUIElementFactory::createGUIButton(testHandler, 150, 150, 30, 30, gui::colorMap.at("GREEN"), &gui::GUIButton::randomColor);
        auto testButtonQuit = gui::GUIElementFactory::createGUIButton(testHandler, 300, 300, 30, 30, gui::colorMap.at("BLUE"), &gui::GUIButton::quitApplication);

        auto testGUIEditTextBase = gui::GUIElementFactory::createGUIElement(testHandler, 300, 40, 200, 100, gui::colorMap.at("RED"));
        auto testGUIEditText = gui::GUIElementFactory::createGUIEditText(testHandler, 0, 0, 200, 100, gui::colorMap.at("BLUE"), L"one\ntwo\nthre3", testFont, true, 1.0f, false);

        auto testGUIEditTextBase2 = gui::GUIElementFactory::createGUIElement(testHandler, 100, 400, 150, 250, gui::colorMap.at("RED"));
        auto testGUIEditText2 = gui::GUIElementFactory::createGUIEditText(testHandler, 0, 0, 150, 250, gui::colorMap.at("BLUE"), L"tes1\ntwo0\ntrez o :)\nmore tex1 is fun!\nanother", testFont2, true, 1.0f, false);

        testElement->addChild(testChild);
        testChild->addChild(testChild2);

        testGUIEditTextBase->addChild(testGUIEditText);
        testGUIEditTextBase2->addChild(testGUIEditText2);

        InputState inputState;

        const float MS_PER_UPDATE = (LOGIC_FREQ_HZ != 0 ? 
                                    1000.0f / LOGIC_FREQ_HZ 
                                    : 1000.0f / 60);
        Uint32 previous = SDL_GetTicks();
        float lag = 0.0;

        float dxMouse = 0, dyMouse = 0;
        bool running = true;
        while (running) 
        {
            Uint32 current = SDL_GetTicks();
            Uint32 elapsed = current - previous;
            previous = current;
            lag += elapsed;

            SDL_Event event;
            while (SDL_PollEvent(&event))
            {   
                if (event.type == SDL_QUIT) 
                {
                    running = false;
                }

                if (event.type == SDL_MOUSEMOTION) 
                {
                    if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT))
                    {
                        dxMouse += event.motion.xrel;
                        dyMouse -= event.motion.yrel;
                    }
                }

                testHandler->handleInputWholeVector(&event, &inputState);
            }

            while (lag >= MS_PER_UPDATE)
            {
                onYawPitch(dxMouse, dyMouse, &inputState);
                onKeys(SDL_GetKeyboardState(NULL), &inputState);
                dxMouse = 0, dyMouse = 0;
                lag -= MS_PER_UPDATE;
            }

            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            drawObject();

            testHandler->renderWholeVector();

            SDL_GL_SwapWindow(window);
        }
    }

    void Renderer::drawObject() 
    {
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::lookAt(camera.cameraPos, camera.targetPos, camera.cameraUp);
        glm::mat4 projection = glm::perspective(FOV, WINDOW_WIDTH / WINDOW_HEIGHT, NEAR_DIST, FAR_DIST);

        // Use the shader program
        glUseProgram(shaderProgram);

        // Set uniforms
        glUniform1i(useTextureLoc, GL_FALSE);
        glUniform3f(objectColorLoc, 1.0f, 1.0f, 1.0f); // White color
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Loop over each mesh in the target object
        for (const auto& mesh : targetObj) 
        {
            // Bind the mesh's VAO
            glBindVertexArray(mesh.VAO);

            // Draw the object
            glDrawElements(GL_TRIANGLES, (GLsizei)mesh.Indices.size(), GL_UNSIGNED_INT, 0);

            // Check for OpenGL errors
            GLenum error = glGetError();
            if (error != GL_NO_ERROR)
            {
                std::cerr << "Error when drawing object, OpenGL error: " << error << std::endl;
            }
        }

        // Unbind the Vertex Array Object
        glBindVertexArray(0);
    }

    void Renderer::onYawPitch(float dx, float dy, InputState* inputState) 
    {
        if (inputState->getMouseState() != InputState::CameraControl)
        {
            return;
        }

        float dxAccum = dx * camera.mouseSensitivity;
        float dyAccum = dy * camera.mouseSensitivity;

        // Update camera yaw and pitch
        camera.cameraYaw += dxAccum;
        camera.cameraPitch += dyAccum;

        // Clamp pitch between -89 and 89 degrees
        if (camera.cameraPitch > glm::radians(89.0f)) 
        {
            camera.cameraPitch = glm::radians(89.0f);
        }
        if (camera.cameraPitch < glm::radians(-89.0f)) 
        {
            camera.cameraPitch = glm::radians(-89.0f);
        }

        // Calculate the new look direction
        glm::vec3 newLookDir;
        newLookDir.x = cos(camera.cameraYaw) * cos(camera.cameraPitch);
        newLookDir.y = sin(camera.cameraPitch);
        newLookDir.z = sin(camera.cameraYaw) * cos(camera.cameraPitch);
        camera.lookDir = glm::normalize(newLookDir);

        // Update target position based on new look direction and camera position
        camera.targetPos = camera.cameraPos + camera.lookDir;
    }

    void Renderer::onKeys(const Uint8* keyboardState, InputState* inputState) 
    {
        if (inputState->getKeyboardState() != InputState::MovementControl)
        {
            return;
        }

        glm::vec3 front = camera.lookDir;
        glm::vec3 right = glm::normalize(glm::cross(camera.cameraUp, front));
        glm::vec3 up = camera.cameraUp;

        // Update camera and target position based on key input
        if (keyboardState[SDL_SCANCODE_W])
        {
            front *= camera.cameraSpeed;
            camera.cameraPos += front;
            camera.targetPos += front;
        }
        if (keyboardState[SDL_SCANCODE_S])
        {
            front *= camera.cameraSpeed;
            camera.cameraPos -= front;
            camera.targetPos -= front;
        }
        if (keyboardState[SDL_SCANCODE_A])
        {
            right *= camera.cameraSpeed;
            camera.cameraPos += right;
            camera.targetPos += right;
        }
        if (keyboardState[SDL_SCANCODE_D])
        {
            right *= camera.cameraSpeed;
            camera.cameraPos -= right;
            camera.targetPos -= right;
        }
        if (keyboardState[SDL_SCANCODE_LSHIFT])
        {
            up *= camera.cameraSpeed;
            camera.cameraPos += up;
            camera.targetPos += up;
        }
        if (keyboardState[SDL_SCANCODE_LCTRL])
        {
            up *= camera.cameraSpeed;
            camera.cameraPos -= up;
            camera.targetPos -= up;
        }
    }

    std::vector<std::string> Renderer::getObjFilePaths(const std::string& folderName) 
    {
        std::vector<std::string> objFiles;
        std::filesystem::path searchPath(folderName);

        // Check if the folder exists in the current directory
        if (!std::filesystem::exists(searchPath)) 
        {
            // If not, check the parent directory
            searchPath = std::filesystem::current_path().parent_path() / folderName;
            
            if (std::filesystem::exists(searchPath))
            {
                // Change the working directory to the parent directory
                std::filesystem::current_path(std::filesystem::current_path().parent_path());

                std::cout << "Changed working directory to: "
                        << std::filesystem::current_path() << std::endl;
            }
        }

        try 
        {
            for (const auto& entry : std::filesystem::directory_iterator(searchPath)) 
            {
                if (entry.is_regular_file() && entry.path().extension() == ".obj") 
                {
                    objFiles.push_back(entry.path().string());
                }
            }
        } 
        catch (const std::filesystem::filesystem_error& e) 
        {
            std::cerr << "Error: " << e.what() << std::endl;
        }

        return objFiles;
    }

    std::vector<objl::Mesh> Renderer::getTargetObjMeshes() 
    {
        objl::Loader loader;
        if (loader.LoadFile(OBJ_PATH + "\\" + targetFile) == 0) 
        {
            std::cout << "Failed to load object file" << std::endl;
        } 
        else 
        {
            std::cout << "Successfully loaded object file" << std::endl;
        }
        return loader.LoadedMeshes;
    }

    void Renderer::nextTargetObj()
    {
        std::vector<std::string> fileNames = allObjPaths;

        // Remove file directory prefix from file names
        for (auto& file : fileNames) 
        {
            std::string prefix = OBJ_PATH + "\\";
            std::size_t found = file.find(prefix);
            
            if (found != std::string::npos) 
            {
                file = file.substr(found + prefix.length());
            }
        }

        int64_t objIx = std::find(fileNames.begin(), fileNames.end(), targetFile) - fileNames.begin();
        assert(objIx != fileNames.size()); // Means file name not found

        if (objIx == fileNames.size() - 1)
        {
            objIx = 0;
        }

        targetFile = fileNames[objIx+1];
        targetObj = getTargetObjMeshes();
    }
}