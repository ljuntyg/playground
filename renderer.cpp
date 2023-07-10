// Credit to Laurent NOËL (celeborn2bealive) for the gltf loader and the rendering 
// code and his tutorial at https://gltf-viewer-tutorial.gitlab.io/ along with the
// accompanying source code at https://gitlab.com/gltf-viewer-tutorial/gltf-viewer

#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <algorithm>

#include "renderer.h"
#include "text.h"
#include "shaders.h"
#include "texture_loader.h"
#define TINYGLTF_IMPLEMENTATION
#include "tiny_gltf.h"
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

namespace renderer
{
    Renderer::Renderer()
    {
        window = nullptr;
        context = nullptr;
        if (SDL_GLAD_init(&window, &context)
            && initializeModel()
            && initializeShaders()
            && initializeCubemaps()
            && initializeGUI())
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
        // This will also delete any GUIElement objects using the guiHandler
        delete guiHandler;

        // Delete defaultFont created using new in getDefaultFont (or delete nullptr if it wasn't created)
        delete text::Font::defaultFont;

        // Clean up shaderProgram, modelTextureIDs, VAOs, VBOs
        glDeleteProgram(shaderProgram);
        for (const auto& texture : modelTextureIDs)
        {
            glDeleteTextures(1, &texture);
        }
        for (const auto& VAO : VAOs)
        {
            glDeleteVertexArrays(1, &VAO);
        }
        for (const auto& VBO : VBOs)
        {
            glDeleteBuffers(1, &VBO);
        }

        // Clean up shaderProgramSkybox, skyboxVAO, skyboxVBO
        glDeleteProgram(shaderProgramSkybox);
        glDeleteVertexArrays(1, &skyboxVAO);
        glDeleteBuffers(1, &skyboxVBO);

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
        else if (const event::NextModelEvent* nextModelEvent = dynamic_cast<const event::NextModelEvent*>(event))
        {
            nextTargetGLTFmodel();
        }
        else if (const event::NextCubemapEvent* nextCubemapEvent = dynamic_cast<const event::NextCubemapEvent*>(event))
        {
            nextTargetCubemap();
        }
        else if (const event::LightAzimuthChangeEvent *lightAzimuthChangeEvent = dynamic_cast<const event::LightAzimuthChangeEvent*>(event))
        {
            lightAzimuth += lightAzimuthChangeEvent->delta;
        }
        else if (const event::LightInclineChangeEvent *lightInclineChangeEvent = dynamic_cast<const event::LightInclineChangeEvent*>(event))
        {
            lightIncline += lightInclineChangeEvent->delta;
        }
        else if (const event::LuminanceChangeEvent* luminanceChangeEvent = dynamic_cast<const event::LuminanceChangeEvent*>(event))
        {
            luminanceFactor += luminanceChangeEvent->delta;
        }
        else if (const event::ScaleChangeEvent *scaleChangeEvent = dynamic_cast<const event::ScaleChangeEvent*>(event))
        {
            scaleFactor += scaleChangeEvent->delta;
        }
        else if (const event::CameraSpeedChangeEvent *cameraSpeedChangeEvent = dynamic_cast<const event::CameraSpeedChangeEvent*>(event))
        {
            camera.cameraSpeed *= cameraSpeedChangeEvent->factor;
        }
    }

    void Renderer::quit()
    {
        running = false;
    }

    bool Renderer::SDL_GLAD_init(SDL_Window** window, SDL_GLContext* context) 
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

        WINDOW_WIDTH = (float)(DM.w / 1.5f);
        WINDOW_HEIGHT = (float)(DM.h / 1.5f);

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);

        *window = SDL_CreateWindow("Playground", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
            (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
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

        std::cout << "Successfully initialized SDL and GLAD" << std::endl;
        return true;
    }

    bool Renderer::initializeModel()
    {
        allGLTFpaths = getGLTFfilePaths(MODELS_PATH);
        if (allGLTFpaths.size() == 0)
        {
            std::cerr << "No gltf files found, object initialization failed" << std::endl;
            return false;
        }

        // If targetGLTFpath has not been initialized, find targetGLTFfile in allGLTFpaths
        // TODO: Use find_if instead of for loop
        if (targetGLTFpath.empty())
        {
            // Create white texture for object with no base color texture on first initialization
            glGenTextures(1, &whiteTextureID);
            glBindTexture(GL_TEXTURE_2D, whiteTextureID);
            float white[] = {1, 1, 1, 1};
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_FLOAT, white);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
            glBindTexture(GL_TEXTURE_2D, 0);

            for (const auto& path : allGLTFpaths)
            {
                if (path.filename().string() == targetGLTFfile)
                {
                    targetGLTFpath = path;
                    break;
                }
            }

            if (targetGLTFpath.empty())
            {
                std::cerr << "Target gltf file not found, object initialization failed" << std::endl;
                return false;
            }
        }

        targetGLTFmodel = tinygltf::Model(); // The model needs to be reset in case this is not the first initialization
        if (!utilgltf::loadGLTFfile(targetGLTFpath, targetGLTFmodel))
        {
            std::cerr << "Failed to load gltf file, object initialization failed" << std::endl;
            return false;
        }
    
        modelTextureIDs = utilgltf::createTextureObjects(targetGLTFmodel);
        VBOs = utilgltf::createVBOs(targetGLTFmodel);
        VAOs = utilgltf::createVAOs(targetGLTFmodel, VBOs, meshToVertexArrays);

        // TODO: Add error checks

        // Compute scene bounds and get diagonal distance
        glm::vec3 bboxMin, bboxMax;
        utilgltf::computeSceneBounds(targetGLTFmodel, bboxMin, bboxMax);
        glm::vec3 diag = bboxMax - bboxMin;
        sceneDiagonalDistance = glm::length(diag);

        // Ensures the scene is within the view frustum (assuming scene is centered at origin)
        NEAR_DIST = (float)0.001 * sceneDiagonalDistance;
        FAR_DIST = (float)100.0 * sceneDiagonalDistance;

        // TODO: Add more error checks after loading the model
        std::cout << "Successfully initialized object" << std::endl;

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

        glUseProgram(shaderProgram);

        modelViewProjMatrixLoc = glGetUniformLocation(shaderProgram, "uModelViewProjMatrix");
        modelViewMatrixLoc = glGetUniformLocation(shaderProgram, "uModelViewMatrix");
        normalMatrixLoc = glGetUniformLocation(shaderProgram, "uNormalMatrix");

        lightDirectionLoc = glGetUniformLocation(shaderProgram, "uLightDirection");
        lightIntensityLoc = glGetUniformLocation(shaderProgram, "uLightIntensity");

        baseColorTextureLoc = glGetUniformLocation(shaderProgram, "uBaseColorTexture");
        baseColorFactorLoc = glGetUniformLocation(shaderProgram, "uBaseColorFactor");

        metallicRoughnessTextureLoc = glGetUniformLocation(shaderProgram, "uMetallicRoughnessTexture");
        metallicFactorLoc = glGetUniformLocation(shaderProgram, "uMetallicFactor");
        roughnessFactorLoc = glGetUniformLocation(shaderProgram, "uRoughnessFactor");

        emissiveTextureLoc = glGetUniformLocation(shaderProgram, "uEmissiveTexture");
        emissiveFactorLoc = glGetUniformLocation(shaderProgram, "uEmissiveFactor");

        occlusionTextureLoc = glGetUniformLocation(shaderProgram, "uOcclusionTexture");
        occlusionStrengthLoc = glGetUniformLocation(shaderProgram, "uOcclusionStrength");
        applyOcclusionLoc = glGetUniformLocation(shaderProgram, "uApplyOcclusion");

        std::cout << "Successfully initialized shaders" << std::endl;
        return true;
    }

    bool Renderer::initializeCubemaps()
    {
        // Create shader program for skybox
        shaderProgramSkybox = shaders::createShaderProgram(shaders::skyboxVertexShaderSource, shaders::skyboxFragmentShaderSource);
        if (shaderProgramSkybox == 0)
        {
            std::cerr << "Failed to create skybox shader program, returning false" << std::endl;
            return false;
        }

        allCubemapPaths = getCubemapPaths(CUBEMAPS_PATH);
        if (allCubemapPaths.size() == 0)
        {
            std::cerr << "No cubemap files found, cubemap initialization failed" << std::endl;
            return false;
        }

        // If targetCubemapPath has not been initialized, find targetCubemapFile in allCubemapPaths
        if (targetCubemapPath.empty())
        {
            auto it = std::find_if(allCubemapPaths.begin(), allCubemapPaths.end(), [this](const std::filesystem::path& path) 
            {
                return path.filename().string() == targetCubemapFile; 
            });

            if (it != allCubemapPaths.end())
            {
                targetCubemapPath = *it;
            }
            else
            {
                std::cerr << "Target cubemap file not found, cubemap initialization failed" << std::endl;
                return false;
            }
        }

        viewSkyboxLoc = glGetUniformLocation(shaderProgramSkybox, "view");
        projectionSkyboxLoc = glGetUniformLocation(shaderProgramSkybox, "projection");

        // Create targetCubemap from targetCubemapPath
        targetCubemap = CubeMap();
        targetCubemap.path = targetCubemapPath;

        float skyboxVertices[] =
        {
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

        // Load cubemap textures
        std::filesystem::path facesCubemap[6] =
        {
            targetCubemap.path / "right.png",
            targetCubemap.path / "left.png",
            targetCubemap.path / "top.png",
            targetCubemap.path / "bottom.png",
            targetCubemap.path / "front.png",
            targetCubemap.path / "back.png"
        };

        glGenTextures(1, &targetCubemap.textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, targetCubemap.textureID);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        // These are very important to prevent seams
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        // This might help with seams on some systems
        //glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

        if (!texturel::loadCubemapTextures(targetCubemap.textureID, facesCubemap))
        {
            std::cerr << "Failed to load cubemap textures, initialization failed" << std::endl;
            return false;
        }

        std::cout << "Successfully initialized cubemaps" << std::endl;
        return true;
    }

    // TODO: Delete fonts? font1 being loaded multiple times
    bool Renderer::initializeGUI()
    {
        guiHandler = new gui::GUIHandler(WINDOW_WIDTH, WINDOW_HEIGHT);

        // Pub-sub
        guiHandler->subscribe(this);
        this->subscribe(guiHandler);

        auto font1 = new text::Font("NotoSans", "res/fonts/NotoSans");
        auto font2 = new text::Font("Coiny", "res/fonts/Coiny");
        auto font3 = new text::Font("Silkscreen", "res/fonts/Silkscreen");

        auto lightInclBase = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(30, 30).setSize(200, 50).setFlags(true, false, true, true).buildElement();
        auto lightInclText = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(45, 0).setSize(110, 50).setFlags(false, false, true, false).setColor(gui::colorMap.at("WHITE")).setText(L"LIGHT\nHEIGHT").setFont(font3).buildText();
        auto lightInclDownButton = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(5, 5).setSize(40, 40).setFlags(false, false, true, true).setColor(gui::colorMap.at("GRAY")).setOnClick(&gui::GUIButton::lightInclineDown).buildButton();
        auto lightInclUpButton = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(155, 5).setSize(40, 40).setFlags(false, false, true, true).setColor(gui::colorMap.at("GRAY")).setOnClick(&gui::GUIButton::lightInclineUp).buildButton();
        auto lightInclDownButtonText = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(2, 5).setSize(40, 80).setFlags(false, false, true, false).setColor(gui::colorMap.at("WHITE")).setText(L"-").setFont(font1).setAutoScaleText(false).setTextScale(1.5f).buildText();
        auto lightInclUpButtonText = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(1, 1).setSize(40, 60).setFlags(false, false, true, false).setColor(gui::colorMap.at("WHITE")).setText(L"+").setFont(font1).setAutoScaleText(false).buildText();

        lightInclBase->addChild(lightInclText);
        lightInclBase->addChild(lightInclUpButton);
        lightInclBase->addChild(lightInclDownButton);
        lightInclUpButton->addChild(lightInclUpButtonText);
        lightInclDownButton->addChild(lightInclDownButtonText);

        auto lightAzimBase = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(30, 90).setSize(200, 50).setFlags(true, false, true, true).buildElement();
        auto lightAzimText = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(45, 0).setSize(110, 50).setFlags(false, false, true, false).setColor(gui::colorMap.at("WHITE")).setText(L"LIGHT\nROTATION").setFont(font3).buildText();
        auto lightAzimDownButton = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(5, 5).setSize(40, 40).setFlags(false, false, true, true).setColor(gui::colorMap.at("GRAY")).setOnClick(&gui::GUIButton::lightAzimuthDown).buildButton();
        auto lightAzimUpButton = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(155, 5).setSize(40, 40).setFlags(false, false, true, true).setColor(gui::colorMap.at("GRAY")).setOnClick(&gui::GUIButton::lightAzimuthUp).buildButton();
        auto lightAzimDownButtonText = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(2, 5).setSize(40, 80).setFlags(false, false, true, false).setColor(gui::colorMap.at("WHITE")).setText(L"-").setFont(font1).setAutoScaleText(false).setTextScale(1.5f).buildText();
        auto lightAzimUpButtonText = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(1, 1).setSize(40, 60).setFlags(false, false, true, false).setColor(gui::colorMap.at("WHITE")).setText(L"+").setFont(font1).setAutoScaleText(false).buildText();

        lightAzimBase->addChild(lightAzimText);
        lightAzimBase->addChild(lightAzimUpButton);
        lightAzimBase->addChild(lightAzimDownButton);
        lightAzimUpButton->addChild(lightAzimUpButtonText);
        lightAzimDownButton->addChild(lightAzimDownButtonText);

        auto luminanceBase = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(30, 150).setSize(200, 50).setFlags(true, false, true, true).buildElement();
        auto luminanceText = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(45, -10).setSize(110, 50).setFlags(false, false, true, false).setColor(gui::colorMap.at("WHITE")).setText(L"BRIGHTNESS").setFont(font3).buildText();
        auto luminanceDownButton = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(5, 5).setSize(40, 40).setFlags(false, false, true, true).setColor(gui::colorMap.at("GRAY")).setOnClick(&gui::GUIButton::luminanceDown).buildButton();
        auto luminanceUpButton = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(155, 5).setSize(40, 40).setFlags(false, false, true, true).setColor(gui::colorMap.at("GRAY")).setOnClick(&gui::GUIButton::luminanceUp).buildButton();
        auto luminanceDownButtonText = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(2, 5).setSize(40, 80).setFlags(false, false, true, false).setColor(gui::colorMap.at("WHITE")).setText(L"-").setFont(font1).setAutoScaleText(false).setTextScale(1.5f).buildText();
        auto luminanceUpButtonText = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(1, 1).setSize(40, 60).setFlags(false, false, true, false).setColor(gui::colorMap.at("WHITE")).setText(L"+").setFont(font1).setAutoScaleText(false).buildText();

        luminanceBase->addChild(luminanceText);
        luminanceBase->addChild(luminanceUpButton);
        luminanceBase->addChild(luminanceDownButton);
        luminanceUpButton->addChild(luminanceUpButtonText);
        luminanceDownButton->addChild(luminanceDownButtonText);

        auto scaleBase = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(30, 210).setSize(200, 50).setFlags(true, false, true, true).buildElement();
        auto scaleText = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(45, 0).setSize(110, 50).setFlags(false, false, true, false).setColor(gui::colorMap.at("WHITE")).setText(L"SCALE").setFont(font3).buildText();
        auto scaleDownButton = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(5, 5).setSize(40, 40).setFlags(false, false, true, true).setColor(gui::colorMap.at("GRAY")).setOnClick(&gui::GUIButton::scaleDown).buildButton();
        auto scaleUpButton = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(155, 5).setSize(40, 40).setFlags(false, false, true, true).setColor(gui::colorMap.at("GRAY")).setOnClick(&gui::GUIButton::scaleUp).buildButton();
        auto scaleDownButtonText = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(2, 5).setSize(40, 80).setFlags(false, false, true, false).setColor(gui::colorMap.at("WHITE")).setText(L"-").setFont(font1).setAutoScaleText(false).setTextScale(1.5f).buildText();
        auto scaleUpButtonText = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(1, 1).setSize(40, 60).setFlags(false, false, true, false).setColor(gui::colorMap.at("WHITE")).setText(L"+").setFont(font1).setAutoScaleText(false).buildText();

        scaleBase->addChild(scaleText);
        scaleBase->addChild(scaleUpButton);
        scaleBase->addChild(scaleDownButton);
        scaleUpButton->addChild(scaleUpButtonText);
        scaleDownButton->addChild(scaleDownButtonText);

        auto cameraSpeedBase = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(30, 270).setSize(200, 50).setFlags(true, false, true, true).buildElement();
        auto cameraSpeedText = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(45, 0).setSize(110, 50).setFlags(false, false, true, false).setColor(gui::colorMap.at("WHITE")).setText(L"CAMERA\nSPEED").setFont(font3).buildText();
        auto cameraSpeedDownButton = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(5, 5).setSize(40, 40).setFlags(false, false, true, true).setColor(gui::colorMap.at("GRAY")).setOnClick(&gui::GUIButton::cameraSpeedDown).buildButton();
        auto cameraSpeedUpButton = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(155, 5).setSize(40, 40).setFlags(false, false, true, true).setColor(gui::colorMap.at("GRAY")).setOnClick(&gui::GUIButton::cameraSpeedUp).buildButton();
        auto cameraSpeedDownButtonText = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(2, 5).setSize(40, 80).setFlags(false, false, true, false).setColor(gui::colorMap.at("WHITE")).setText(L"-").setFont(font1).setAutoScaleText(false).setTextScale(1.5f).buildText();
        auto cameraSpeedUpButtonText = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(1, 1).setSize(40, 60).setFlags(false, false, true, false).setColor(gui::colorMap.at("WHITE")).setText(L"+").setFont(font1).setAutoScaleText(false).buildText();

        cameraSpeedBase->addChild(cameraSpeedText);
        cameraSpeedBase->addChild(cameraSpeedUpButton);
        cameraSpeedBase->addChild(cameraSpeedDownButton);
        cameraSpeedUpButton->addChild(cameraSpeedUpButtonText);
        cameraSpeedDownButton->addChild(cameraSpeedDownButtonText);

        auto nextModelButton = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(30, 330).setSize(200, 50).setFlags(true, false, true, true).setOnClick(&gui::GUIButton::nextModel).buildButton();
        auto nextModelText = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(0, 0).setSize(200, 50).setFlags(false, false, true, false).setColor(gui::colorMap.at("WHITE")).setText(L"NEXT MODEL").setFont(font3).setPadding(10).buildText();

        nextModelButton->addChild(nextModelText);

        auto nextCubemapButton = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(30, 390).setSize(200, 50).setFlags(true, false, true, true).setOnClick(&gui::GUIButton::nextCubemap).buildButton();
        auto nextCubemapText = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(0, 0).setSize(200, 50).setFlags(false, false, true, false).setColor(gui::colorMap.at("WHITE")).setText(L"NEXT CUBEMAP").setFont(font3).setPadding(10).buildText();

        nextCubemapButton->addChild(nextCubemapText);

        auto playgroundBase = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(240, 30).setSize(155, 35).setColor(gui::colorMap.at("BLUE")).buildElement();
        auto playgroundChild = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(10, 10).setSize(155, 35).setFlags(false, false, true, false).setColor(gui::colorMap.at("RED")).buildElement();
        auto playgroundChild2 = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(10, 10).setSize(155, 35).setFlags(false, false, true, false).setColor(gui::colorMap.at("YELLOW")).buildElement();
        auto playgroundChild2GUIEditText = gui::GUIElementBuilder().setHandler(guiHandler).setPosition(0, 0).setSize(155, 35).setFlags(false, false, true, false).setColor(gui::colorMap.at("BLACK")).setText(L"PLAYGROUND").setFont(font2).buildEditText();

        playgroundBase->addChild(playgroundChild);
        playgroundChild->addChild(playgroundChild2);
        playgroundChild2->addChild(playgroundChild2GUIEditText);

        if (guiHandler->getZIndexRootElementMap().size() == 0)
        {
            std::cout << "Warning: guiHandler has no elements" << std::endl;
        }

        std::cout << "Successfully initialized GUI" << std::endl;
        return true;
    }

    void Renderer::run()
    {
        const float MS_PER_UPDATE = (LOGIC_FREQ_HZ != 0 ? 
                                    1000.0f / LOGIC_FREQ_HZ 
                                    : 1000.0f / 60);
        Uint32 previous = SDL_GetTicks();
        float lag = 0.0;

        float targetYaw = 0, targetPitch = 0;
        while (running) 
        {
            Uint32 current = SDL_GetTicks();
            Uint32 elapsed = current - previous;
            previous = current;
            lag += elapsed;

            SDL_Event event;
            while (SDL_PollEvent(&event))
            {   
                guiHandler->receiveInputAllElements(&event, &inputState);

                if (event.type == SDL_QUIT) 
                {
                    running = false;
                }

                if (event.type == SDL_MOUSEMOTION) 
                {
                    if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)
                        && inputState.getMouseState() == InputState::CameraControl)
                    {
                        targetYaw += event.motion.xrel * camera.horizontalMouseSensitivity;
                        targetPitch -= event.motion.yrel * camera.verticalMouseSensitivity;
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
                        WINDOW_WIDTH = (float)newWidth, WINDOW_HEIGHT = (float)newHeight;
                    }
                }
            }

            while (lag >= MS_PER_UPDATE)
            {
                onYawPitch(targetYaw, targetPitch, &inputState);
                onKeys(SDL_GetKeyboardState(NULL), &inputState);
                lag -= MS_PER_UPDATE;
            }

            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            viewMatrix = glm::lookAt(camera.cameraPos, camera.targetPos, camera.cameraUp);
            projMatrix = glm::perspective(FOV, WINDOW_WIDTH / WINDOW_HEIGHT, NEAR_DIST, FAR_DIST); // Just in case of window resize

            drawSkybox();

            drawModel();

            guiHandler->renderAllElements();

            SDL_GL_SwapWindow(window);
        }
    }

    void Renderer::onYawPitch(float targetYaw, float targetPitch, InputState* inputState) 
    {
        if (inputState->getMouseState() != InputState::CameraControl)
        {
            return;
        }

        camera.cameraYaw += (targetYaw - camera.cameraYaw) * camera.smoothingFactor;
        camera.cameraPitch += (targetPitch - camera.cameraPitch) * camera.smoothingFactor;

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

    void Renderer::drawModel()
    {
        // Use the shader program
        glUseProgram(shaderProgram);

        // Init light parameters
        glm::vec3 lightDirection = glm::vec3(std::sin(lightIncline) * std::cos(lightAzimuth), std::sin(lightIncline) * std::sin(lightAzimuth), std::cos(lightIncline));
        glm::vec3 lightIntensity = glm::vec3(1, 1, 1) * luminanceFactor;
        bool lightFromCamera = false;
        bool applyOcclusion = true;

        if (lightDirectionLoc >= 0) 
        {
            if (lightFromCamera) 
            {
                glUniform3f(lightDirectionLoc, 0, 0, 1);
            } 
            else 
            {
                const auto lightDirectionInViewSpace = glm::normalize(glm::vec3(viewMatrix * glm::vec4(lightDirection, 0.)));
                glUniform3f(lightDirectionLoc, lightDirectionInViewSpace[0], lightDirectionInViewSpace[1], lightDirectionInViewSpace[2]);
            }
        }

        if (lightIntensityLoc >= 0) 
        {
            glUniform3f(lightIntensityLoc, lightIntensity[0], lightIntensity[1], lightIntensity[2]);
        }

        if (applyOcclusionLoc >= 0) 
        {
            glUniform1i(applyOcclusionLoc, applyOcclusion);
        }

        // Draw model by initiating recursive drawNode calls
        if (targetGLTFmodel.defaultScene >= 0) 
        {
            for (const auto nodeIdx : targetGLTFmodel.scenes[targetGLTFmodel.defaultScene].nodes) 
            {
                drawNode(nodeIdx, glm::mat4(1), true);
            }
        }
    }

    void Renderer::drawNode(int nodeIdx, const glm::mat4 parentMatrix, bool isRoot)
    {
        const auto& node = targetGLTFmodel.nodes[nodeIdx];
        glm::mat4 modelMatrix = utilgltf::getLocalToWorldMatrix(node, parentMatrix);

        if (isRoot) // Apply scale factor to the model at the root level only
        {
            modelMatrix = glm::scale(modelMatrix, glm::vec3(scaleFactor));
        }

        // If the node references a mesh (a node can also reference a
        // camera, or a light)
        if (node.mesh >= 0) 
        {
            const auto mvMatrix = viewMatrix * modelMatrix; // Also called localToCamera matrix
            const auto mvpMatrix = projMatrix * mvMatrix; // Also called localToScreen matrix
            // Normal matrix is necessary to maintain normal vectors
            // orthogonal to tangent vectors
            // https://www.lighthouse3d.com/tutorials/glsl-12-tutorial/the-normal-matrix/
            const auto normalMatrix = glm::transpose(glm::inverse(mvMatrix));

            glUniformMatrix4fv(modelViewProjMatrixLoc, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
            glUniformMatrix4fv(modelViewMatrixLoc, 1, GL_FALSE, glm::value_ptr(mvMatrix));
            glUniformMatrix4fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

            const auto& mesh = targetGLTFmodel.meshes[node.mesh];
            const auto& vaoRange = meshToVertexArrays[node.mesh];
            for (size_t pIdx = 0; pIdx < mesh.primitives.size(); ++pIdx) 
            {
                const auto vao = VAOs[vaoRange.begin + pIdx];
                const auto &primitive = mesh.primitives[pIdx];

                bindMaterial(primitive.material);

                glBindVertexArray(vao);
                if (primitive.indices >= 0) 
                {
                    const auto& accessor = targetGLTFmodel.accessors[primitive.indices];
                    const auto& bufferView = targetGLTFmodel.bufferViews[accessor.bufferView];
                    const auto byteOffset = accessor.byteOffset + bufferView.byteOffset;
                    glDrawElements(primitive.mode, GLsizei(accessor.count), accessor.componentType, (const GLvoid *)byteOffset);
                } 
                else 
                {
                    // Take first accessor to get the count
                    const auto accessorIdx = (*begin(primitive.attributes)).second;
                    const auto& accessor = targetGLTFmodel.accessors[accessorIdx];
                    glDrawArrays(primitive.mode, 0, GLsizei(accessor.count));
                }
            }
        }

        // Draw children
        for (const auto childNodeIdx : node.children) 
        {
            drawNode(childNodeIdx, modelMatrix);
        }
    }

    void Renderer::bindMaterial(const int materialIndex)
    {
        if (materialIndex >= 0) 
        {
            const auto& material = targetGLTFmodel.materials[materialIndex];
            const auto& pbrMetallicRoughness = material.pbrMetallicRoughness;
            if (baseColorFactorLoc >= 0) 
            {
                glUniform4f(baseColorFactorLoc,
                    (float)pbrMetallicRoughness.baseColorFactor[0],
                    (float)pbrMetallicRoughness.baseColorFactor[1],
                    (float)pbrMetallicRoughness.baseColorFactor[2],
                    (float)pbrMetallicRoughness.baseColorFactor[3]);
            }
            if (baseColorTextureLoc >= 0) 
            {
                auto textureObject = whiteTextureID;
                if (pbrMetallicRoughness.baseColorTexture.index >= 0) {
                    const auto &texture = targetGLTFmodel.textures[pbrMetallicRoughness.baseColorTexture.index];
                    if (texture.source >= 0) 
                    {
                        textureObject = modelTextureIDs[texture.source];
                    }
                }

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, textureObject);
                glUniform1i(baseColorTextureLoc, 0);
            }
            if (metallicFactorLoc >= 0) 
            {
                glUniform1f(metallicFactorLoc, (float)pbrMetallicRoughness.metallicFactor);
            }
            if (roughnessFactorLoc >= 0)
            {
                glUniform1f(roughnessFactorLoc, (float)pbrMetallicRoughness.roughnessFactor);
            }
            if (metallicRoughnessTextureLoc >= 0) 
            {
                auto textureObject = 0u;
                if (pbrMetallicRoughness.metallicRoughnessTexture.index >= 0) 
                {
                    const auto &texture = targetGLTFmodel.textures[pbrMetallicRoughness.metallicRoughnessTexture.index];
                    if (texture.source >= 0) 
                    {
                        textureObject = modelTextureIDs[texture.source];
                    }
                }

                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, textureObject);
                glUniform1i(metallicRoughnessTextureLoc, 1);
            }
            if (emissiveFactorLoc >= 0)
            {
                glUniform3f(emissiveFactorLoc, (float)material.emissiveFactor[0],
                    (float)material.emissiveFactor[1],
                    (float)material.emissiveFactor[2]);
            }
            if (emissiveTextureLoc >= 0) 
            {
                auto textureObject = 0u;
                if (material.emissiveTexture.index >= 0) 
                {
                    const auto &texture = targetGLTFmodel.textures[material.emissiveTexture.index];
                    if (texture.source >= 0) 
                    {
                        textureObject = modelTextureIDs[texture.source];
                    }
                }

                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, textureObject);
                glUniform1i(emissiveTextureLoc, 2);
            }
            if (occlusionStrengthLoc >= 0) 
            {
                glUniform1f(occlusionStrengthLoc, (float)material.occlusionTexture.strength);
            }
            if (occlusionTextureLoc >= 0) 
            {
                auto textureObject = whiteTextureID;
                if (material.occlusionTexture.index >= 0) 
                {
                    const auto &texture = targetGLTFmodel.textures[material.occlusionTexture.index];
                    if (texture.source >= 0) 
                    {
                        textureObject = modelTextureIDs[texture.source];
                    }
                }

                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, textureObject);
                glUniform1i(occlusionTextureLoc, 3);
            }
        } 
        else 
        {
            // Apply default material
            // Defined here:
            // https://github.com/KhronosGroup/glTF/blob/master/specification/2.0/README.md#reference-material
            // https://github.com/KhronosGroup/glTF/blob/master/specification/2.0/README.md#reference-pbrmetallicroughness3
            if (baseColorFactorLoc >= 0)
            {
                glUniform4f(baseColorFactorLoc, 1, 1, 1, 1);
            }
            if (baseColorTextureLoc >= 0)
            {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, whiteTextureID);
                glUniform1i(baseColorTextureLoc, 0);
            }
            if (metallicFactorLoc >= 0) 
            {
                glUniform1f(metallicFactorLoc, 1.f);
            }
            if (roughnessFactorLoc >= 0)
            {
                glUniform1f(roughnessFactorLoc, 1.f);
            }
            if (metallicRoughnessTextureLoc >= 0)
            {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, 0);
                glUniform1i(metallicRoughnessTextureLoc, 1);
            }
            if (emissiveFactorLoc >= 0)
            {
                glUniform3f(emissiveFactorLoc, 0.f, 0.f, 0.f);
            }
            if (emissiveTextureLoc >= 0)
            {
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, 0);
                glUniform1i(emissiveTextureLoc, 2);
            }
            if (occlusionStrengthLoc >= 0)
            {
                glUniform1f(occlusionStrengthLoc, 0.f);
            }
            if (occlusionTextureLoc >= 0)
            {
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, 0);
                glUniform1i(occlusionTextureLoc, 3);
            }
        }
    }

    void Renderer::drawSkybox() 
    {
        if (targetCubemapPath.empty()) 
        {
            return;
        }

        glDisable(GL_CULL_FACE);
        glDepthFunc(GL_LEQUAL); // Change depth function so depth test passes when values are equal to depth buffer's content

        glUseProgram(shaderProgramSkybox);  // Use skybox shader

        glUniformMatrix4fv(viewSkyboxLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4(glm::mat3(viewMatrix))));
        glUniformMatrix4fv(projectionSkyboxLoc, 1, GL_FALSE, glm::value_ptr(projMatrix));

        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, targetCubemap.textureID);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        // Check for OpenGL errors
        GLenum error = glGetError();
        if (error != GL_NO_ERROR)
        {
            std::cerr << "Error when drawing skybox, OpenGL error: " << error << std::endl;
        }

        glBindVertexArray(0);

        glDepthFunc(GL_LESS); // Set depth function back to default
        glEnable(GL_CULL_FACE);
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
                    cubemapDirs.push_back(std::filesystem::absolute(entry.path()));
                }
            }
        } 
        catch (const std::filesystem::filesystem_error& e) 
        {
            std::cerr << "Error: " << e.what() << std::endl;
        }

        return cubemapDirs;
    }

    void Renderer::nextTargetCubemap()
    {
        auto it = std::find(allCubemapPaths.begin(), allCubemapPaths.end(), targetCubemapPath);
        if(it != allCubemapPaths.end())
        {
            ++it;
            // Once on last cubemap, go to no cubemap, then next time go to first cubemap
            if(it == allCubemapPaths.end())
            {
                if (skyboxVAO)
                {
                    glDeleteVertexArrays(1, &skyboxVAO);
                    skyboxVAO = 0;
                }
                if (skyboxVBO)
                {
                    glDeleteBuffers(1, &skyboxVBO);
                    skyboxVBO = 0;
                }
                if (skyboxEBO)
                {
                    glDeleteBuffers(1, &skyboxEBO);
                    skyboxEBO = 0;
                }

                targetCubemapPath = std::filesystem::path();
                return;
            }
        }
        else // Cubemap not found means we're on no cubemap, so go to first cubemap
        {
            it = allCubemapPaths.begin();
        }

        targetCubemapPath = *it;

        if (skyboxVAO)
        {
            glDeleteVertexArrays(1, &skyboxVAO);
            skyboxVAO = 0;
        }
        if (skyboxVBO)
        {
            glDeleteBuffers(1, &skyboxVBO);
            skyboxVBO = 0;
        }
        if (skyboxEBO)
        {
            glDeleteBuffers(1, &skyboxEBO);
            skyboxEBO = 0;
        }

        initializeCubemaps();
    }

    std::vector<std::filesystem::path> Renderer::getGLTFfilePaths(std::string folderName) 
    {
        std::vector<std::filesystem::path> gltfFiles;
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
                if (entry.is_regular_file()
                    && (entry.path().extension() == ".glb" || entry.path().extension() == ".gltf")) 
                {
                    gltfFiles.push_back(std::filesystem::absolute(entry.path()));
                }
            }
        } 
        catch (const std::filesystem::filesystem_error& e) 
        {
            std::cerr << "Error: " << e.what() << std::endl;
        }

        return gltfFiles;
    }

    void Renderer::nextTargetGLTFmodel()
    {
        auto it = std::find(allGLTFpaths.begin(), allGLTFpaths.end(), targetGLTFpath);
        if(it != allGLTFpaths.end())
        {
            ++it;
            if(it == allGLTFpaths.end())
            {
                it = allGLTFpaths.begin();
            }
        }
        else
        {
            std::cout << "Warning: Current targetGLTFpath not found, this isn't intended behavior, set targetGLTFpath to first in allGLTFpaths" << std::endl;
            it = allGLTFpaths.begin();
        }

        targetGLTFpath = *it;

        if (!modelTextureIDs.empty()) 
        {
            glDeleteTextures(GLsizei(modelTextureIDs.size()), modelTextureIDs.data());
            modelTextureIDs.clear();
        }
        if (!VBOs.empty()) 
        {
            glDeleteBuffers(GLsizei(VBOs.size()), VBOs.data());
            VBOs.clear();
        }
        if (!VAOs.empty()) 
        {
            glDeleteVertexArrays(GLsizei(VAOs.size()), VAOs.data());
            VAOs.clear();
        }
        if (!meshToVertexArrays.empty()) 
        {
            meshToVertexArrays.clear();
        }

        initializeModel();

        camera.cameraPos = glm::vec3(0, 0, 5);
        camera.targetPos = glm::vec3(0, 0, 0);
    }
}