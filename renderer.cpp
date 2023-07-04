#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <algorithm>

#include "renderer.h"
#include "text.h"
#include "shaders.h"
#include "gui.h"
#include "texture_loader.h"

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

        *WINDOW_WIDTH = (float)(DM.w / 1.5f);
        *WINDOW_HEIGHT = (float)(DM.h / 1.5f);

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

        *window = SDL_CreateWindow("Playground", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
            (int)*WINDOW_WIDTH, (int)*WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
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
        if (SDL_GLAD_init(&window, &context, &WINDOW_WIDTH, &WINDOW_HEIGHT)
            && initializeObject()
            && initializeShaders()
            && initializeCubemaps())
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
        // Delete defaultFont created using new in getDefaultFont (or delete nullptr if it wasn't created)
        delete text::Font::defaultFont;

        // Clean up shaderProgram, VAO, VBO, EBO
        glDeleteProgram(shaderProgram);
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);

        // Clean up shaderProgramSkybox, skyboxVAO, skyboxVBO
        glDeleteProgram(shaderProgramSkybox);
        glDeleteVertexArrays(1, &skyboxVAO);
        glDeleteBuffers(1, &skyboxVBO);

        // Clean up cubemaps
        for (auto& cubemap : cubemaps)
        {
            glDeleteTextures(1, &(cubemap->textureID));
            delete cubemap;
        }

        // SDL clean up
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void Renderer::notify(const event::Event* event) 
    {
        if (const event::QuitEvent* quitEvent = dynamic_cast<const event::QuitEvent*>(event)) 
        {
            quit();
        }
    }

    void Renderer::quit()
    {
        running = false;
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

    // Skybox buffers are initialized in initializeCubemaps
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

        std::cout << "Successfully initialized shaders" << std::endl;
        return true;
    }

    // TODO: Rethink/fix
    bool Renderer::initializeCubemaps()
    {
        // Create shader program for skybox
        shaderProgramSkybox = shaders::createShaderProgram(shaders::skyboxVertexShaderSource, shaders::skyboxFragmentShaderSource);
        if (shaderProgramSkybox == 0)
        {
            std::cerr << "Failed to create skybox shader program, returning false" << std::endl;
            return false;
        }

        // Load paths to cubemaps
        std::vector<std::filesystem::path> cubemapPaths = getCubemapPaths(CUBEMAPS_PATH);
        if (cubemapPaths.size() == 0)
        {
            std::cerr << "No cubemap files found, no further initialization, but returning true" << std::endl;
            return true;
        }
        else for (const auto& path : cubemapPaths)
        {
            CubeMap* cubemap = new CubeMap();
            cubemap->path = path;
            cubemap->textureID = 0;
            cubemaps.push_back(cubemap);
        }

        float skyboxVertices[] =
        {
            //   Coordinates
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f
        };

        unsigned int skyboxIndices[] =
        {
            // Right
            1, 2, 6,
            6, 5, 1,
            // Left
            0, 4, 7,
            7, 3, 0,
            // Top
            4, 5, 6,
            6, 7, 4,
            // Bottom
            0, 3, 2,
            2, 1, 0,
            // Back
            0, 1, 5,
            5, 4, 0,
            // Front
            3, 7, 6,
            6, 2, 3
        };

        // Create VAO, VBO, EBO for skybox
        glGenVertexArrays(1, &skyboxVAO);
        glGenBuffers(1, &skyboxVBO);
        glGenBuffers(1, &skyboxEBO);
        glBindVertexArray(skyboxVAO);
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        // Find target cubemap in cubemapPaths
        for (const auto& cm : cubemaps)
        {
            if (cm->path.filename().string() == targetCubemapFile)
            {
                targetCubemap = cm;
                break;
            }            
        }
        if (targetCubemap == nullptr)
        {
            std::cerr << "Target cubemap file not found, initialization failed" << std::endl;
            return false;
        }

        // Load cubemap textures
        std::filesystem::path facesCubemap[6] =
        {
            targetCubemap->path / "right.png",
            targetCubemap->path / "left.png",
            targetCubemap->path / "top.png",
            targetCubemap->path / "bottom.png",
            targetCubemap->path / "front.png",
            targetCubemap->path / "back.png"
        };

        glGenTextures(1, &targetCubemap->textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, targetCubemap->textureID);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        // These are very important to prevent seams
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        // This might help with seams on some systems
        //glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

        if (!texturel::loadCubemapTextures(targetCubemap->textureID, facesCubemap))
        {
            std::cerr << "Failed to load cubemap textures, initialization failed" << std::endl;
            return false;
        }

        std::cout << "Successfully initialized cubemaps" << std::endl;
        return true;
    }

    void Renderer::run()
    {
        auto testFont = new text::Font("Coiny", "res/fonts/Coiny");
        auto testFont2 = new text::Font("DukePlus", "res/fonts/DukePlus");
        auto testFont3 = new text::Font("KuaiLe", "res/fonts/ZCOOLKuaiLe");
        auto testFont4 = new text::Font("VT323", "res/fonts/VT323");
        auto testHandler = new gui::GUIHandler(WINDOW_WIDTH, WINDOW_HEIGHT);

        // Pub-sub
        testHandler->subscribe(this);
        this->subscribe(testHandler);

        auto testElement = gui::GUIElementBuilder().setHandler(testHandler).setPosition(30, 30).setSize(55, 55).setColor(gui::colorMap.at("BLUE")).buildElement();
        auto testChild = gui::GUIElementBuilder().setHandler(testHandler).setPosition(10, 10).setSize(55, 55).setFlags(false, false, true, false).setColor(gui::colorMap.at("RED")).buildElement();
        auto testChild2 = gui::GUIElementBuilder().setHandler(testHandler).setPosition(10, 10).setSize(55, 55).setFlags(false, false, true, false).setColor(gui::colorMap.at("YELLOW")).buildElement();
        auto testChild2GUIText = gui::GUIElementBuilder().setHandler(testHandler).setPosition(0, 0).setSize(55, 55).setFlags(false, false, true, false).setColor(gui::colorMap.at("BLACK")).setText(L"PLAYGROUND!").setFont(testFont).buildEditText();

        auto testButton = gui::GUIElementBuilder().setHandler(testHandler).setPosition(30, 120).setSize(30, 30).setColor(gui::colorMap.at("GREEN")).setOnClick(&gui::GUIButton::randomColor).buildButton();
        auto testButtonQuit = gui::GUIElementBuilder().setHandler(testHandler).setPosition(30, 160).setSize(30, 30).setColor(gui::colorMap.at("RED")).setOnClick(&gui::GUIButton::quitApplication).buildButton();
        auto testButtonQuitText = gui::GUIElementBuilder().setHandler(testHandler).setPosition(0, 0).setSize(30, 30).setFlags(false, false, true, false).setColor(gui::colorMap.at("WHITE")).setText(L"Quit").setFont(testFont).buildText();
        auto testButton2Base = gui::GUIElementBuilder().setHandler(testHandler).setPosition(30, 200).setSize(40, 40).buildElement();
        auto testButton2 = gui::GUIElementBuilder().setHandler(testHandler).setPosition(5, 5).setSize(30, 30).setFlags(false, false, true, true).setColor(gui::colorMap.at("GREEN")).setOnClick(&gui::GUIButton::randomColor).buildButton();

        auto testGUIEditTextBase = gui::GUIElementBuilder().setHandler(testHandler).setPosition(130, 30).setSize(75, 75).buildElement();
        auto testGUIEditText = gui::GUIElementBuilder().setHandler(testHandler).setPosition(0, 0).setSize(75, 75).setFlags(false, false, true, false).setColor(gui::colorMap.at("WHITE")).setText(L"Text!").setPadding(10).setFont(testFont2).buildEditText();

        auto testGUIEditTextBase2 = gui::GUIElementBuilder().setHandler(testHandler).setPosition(230, 30).setSize(75, 75).buildElement();
        auto testGUIEditText2 = gui::GUIElementBuilder().setHandler(testHandler).setPosition(0, 0).setSize(75, 75).setFlags(false, false, true, false).setColor(gui::colorMap.at("WHITE")).setText(L"Text!").setPadding(10).setFont(testFont3).buildEditText();

        auto testGUIEditTextBase3 = gui::GUIElementBuilder().setHandler(testHandler).setPosition(330, 30).setSize(75, 75).buildElement();
        auto testGUIEditText3 = gui::GUIElementBuilder().setHandler(testHandler).setPosition(0, 0).setSize(75, 75).setFlags(false, false, true, false).setColor(gui::colorMap.at("WHITE")).setText(L"Text!").setPadding(10).setFont(testFont4).buildEditText();

        testElement->addChild(testChild);
        testChild->addChild(testChild2);
        testChild2->addChild(testChild2GUIText);

        testButtonQuit->addChild(testButtonQuitText);
        testButton2Base->addChild(testButton2);

        testGUIEditTextBase->addChild(testGUIEditText);
        testGUIEditTextBase2->addChild(testGUIEditText2);
        testGUIEditTextBase3->addChild(testGUIEditText3);

        InputState inputState;

        const float MS_PER_UPDATE = (LOGIC_FREQ_HZ != 0 ? 
                                    1000.0f / LOGIC_FREQ_HZ 
                                    : 1000.0f / 60);
        Uint32 previous = SDL_GetTicks();
        float lag = 0.0;

        float dxMouse = 0, dyMouse = 0;
        while (running) 
        {
            Uint32 current = SDL_GetTicks();
            Uint32 elapsed = current - previous;
            previous = current;
            lag += elapsed;

            SDL_Event event;
            while (SDL_PollEvent(&event))
            {   
                testHandler->receiveInputAllElements(&event, &inputState);

                if (event.type == SDL_QUIT) 
                {
                    running = false;
                }

                if (event.type == SDL_MOUSEMOTION) 
                {
                    if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)
                        && inputState.getMouseState() == InputState::CameraControl)
                    {
                        dxMouse += event.motion.xrel;
                        dyMouse -= event.motion.yrel;
                    }
                }

                if (event.type == SDL_WINDOWEVENT) 
                {
                    if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) 
                    {
                        int newWidth = event.window.data1;
                        int newHeight = event.window.data2;
                        glViewport(0, 0, newWidth, newHeight);
                        this->publish(new event::WindowResizeEvent(newWidth, newHeight));
                    }
                }
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

            drawSkybox();

            drawObject();

            testHandler->renderAllElements();

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

    // TODO: Rethink/fix
    void Renderer::drawSkybox() 
    {
        if (cubemaps.empty())
        {
            std::cerr << "No cubemaps found, not drawing skybox" << std::endl;
            return;
        }

        glDisable(GL_CULL_FACE);
        glDepthFunc(GL_LEQUAL);  // Change depth function so depth test passes when values are equal to depth buffer's content

        glm::mat4 view = glm::mat4(glm::mat3(glm::lookAt(camera.cameraPos, camera.targetPos, camera.cameraUp)));
        glm::mat4 projection = glm::perspective(FOV, WINDOW_WIDTH / WINDOW_HEIGHT, NEAR_DIST, FAR_DIST);

        glUseProgram(shaderProgramSkybox);  // Use skybox shader

        auto viewSkyboxLoc = glGetUniformLocation(shaderProgramSkybox, "view");
        auto projectionSkyboxLoc = glGetUniformLocation(shaderProgramSkybox, "projection");
        glUniformMatrix4fv(viewSkyboxLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionSkyboxLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(skyboxVAO);  // skybox cube
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, targetCubemap->textureID);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        // Check for OpenGL errors
        GLenum error = glGetError();
        if (error != GL_NO_ERROR)
        {
            std::cerr << "Error when drawing skybox, OpenGL error: " << error << std::endl;
        }

        glBindVertexArray(0);

        glDepthFunc(GL_LESS);  // Set depth function back to default
        glEnable(GL_CULL_FACE);
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

    std::vector<std::string> Renderer::getObjFilePaths(std::string folderName) 
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
        if (loader.LoadFile(OBJ_PATH + "\\" + targetObjFile) == 0) 
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

        int64_t objIx = std::find(fileNames.begin(), fileNames.end(), targetObjFile) - fileNames.begin();
        assert(objIx != fileNames.size()); // Means file name not found

        if (objIx == fileNames.size() - 1)
        {
            objIx = 0;
        }

        targetObjFile = fileNames[objIx+1];
        targetObj = getTargetObjMeshes();
    }

    std::vector<std::filesystem::path> Renderer::getCubemapPaths(std::string folderName) 
    {
        std::vector<std::filesystem::path> cubemapDirs;
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
                if (entry.is_directory()) 
                {
                    cubemapDirs.push_back(entry.path());
                }
            }
        } 
        catch (const std::filesystem::filesystem_error& e) 
        {
            std::cerr << "Error: " << e.what() << std::endl;
        }

        return cubemapDirs;
    }
}