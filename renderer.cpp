#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <algorithm>

#include "renderer.h"

namespace renderer
{
    int SDL_GLAD_init(SDL_Window* window, SDL_GLContext* context) 
    {
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
            return -1;
        }

        SDL_DisplayMode DM;
        if (SDL_GetDesktopDisplayMode(0, &DM) != 0)
        {
            std::cerr << "Failed to get display mode: " << SDL_GetError() << std::endl;
            return 1;
        }

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

        window = SDL_CreateWindow("Playground", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
            DM.w, DM.h, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP);
        if (window == nullptr)
        {
            std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
            return -1;
        }

        *context = SDL_GL_CreateContext(window);
        if (context == nullptr)
        {
            std::cerr << "Failed to create OpenGL context: " << SDL_GetError() << std::endl;
            return -1;
        }

        if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
        {
            std::cerr << "Failed to initialize GLAD" << std::endl;
            return -1;
        }

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        return 0;
    }

    // Function to compile a shader, helper method to createShaderProgram
    GLuint compileShader(const GLenum type, const GLchar *source)
    {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);

        GLint status;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if (!status)
        {
            GLchar infoLog[512];
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            std::cerr << "Failed to compile shader: " << infoLog << std::endl;
        }

        return shader;
    }

    // Function to create a shader program
    GLuint createShaderProgram(const GLchar *vertexShaderSource, const GLchar *fragmentShaderSource)
    {
        GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
        GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

        GLuint program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        GLint status;
        glGetProgramiv(program, GL_LINK_STATUS, &status);
        if (!status)
        {
            GLchar infoLog[512];
            glGetProgramInfoLog(program, 512, nullptr, infoLog);
            std::cerr << "Failed to link shader program: " << infoLog << std::endl;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return program;
    }

    Renderer::Renderer()
    {
        window = nullptr;
        context = nullptr;
        if (SDL_GLAD_init(window, &context) != 0) 
        {
            RENDERER_STATE = RENDERER_CREATE_ERROR;
            return;
        } 
        else 
        {
            RENDERER_STATE = RENDERER_CREATED;
        }

        allObjNames = getObjFileNames(OBJ_PATH);
        targetObj = getTargetObjMesh();

        shaderProgram = createShaderProgram(rendererVertexShaderSource, rendererFragmentShaderSource);
        glUseProgram(shaderProgram);
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

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

    void Renderer::run()
    {
        // Event loop
        bool running = true;
        while (running) 
        {
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT) 
                {
                    running = false;
                }
            }

            drawObject();
        }
    }

    void Renderer::drawObject()
    {
       
    }

    void Renderer::onYawPitch(float dx, float dy) 
    {
        // Update camera yaw and pitch
        camera.cameraYaw += dx;
        camera.cameraPitch += dy;

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

    void Renderer::onKeys(const int& key) 
    {
        glm::vec3 front = camera.lookDir;
        glm::vec3 right = glm::normalize(glm::cross(camera.cameraUp, front));
        glm::vec3 up = camera.cameraUp;

        float sign = std::pow(-1, key);

        if (key == 0 || key == 1) // Forward/backward
        {
            front *= camera.cameraSpeed;
            camera.cameraPos += sign * front;
            camera.targetPos += sign * front;
        }
        if (key == 2 || key == 3) // Right/left
        {
            right *= camera.cameraSpeed;
            camera.cameraPos += sign * right;
            camera.targetPos += sign * right;
        }
        if (key == 4 || key == 5) // Up/down
        {
            up *= camera.cameraSpeed;
            camera.cameraPos += sign * up;
            camera.targetPos += sign * up;
        }
    }

    std::vector<std::string> Renderer::getObjFileNames(const std::string& folderName) 
    {
        std::vector<std::string> objFiles;

        try 
        {
            for (const auto& entry : std::filesystem::directory_iterator(folderName)) 
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

    std::vector<objl::Mesh> Renderer::getTargetObjMesh() 
    {
        objl::Loader loader;
        if (loader.LoadFile(OBJ_PATH + "/" + targetFile) == 0) 
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
        std::vector<std::string> fileNames = allObjNames;

        // Remove file directory prefix from file names
        for (auto& file : fileNames) 
        {
            std::string prefix = OBJ_PATH + "\\";
            std::size_t found = file.find(prefix);
            
            if (found != std::string::npos) {
                file = file.substr(found + prefix.length());
            }
        }

        int objIx = std::find(fileNames.begin(), fileNames.end(), targetFile) - fileNames.begin();
        assert(objIx != fileNames.size()); // Means file name not found

        if (objIx == fileNames.size() - 1)
        {
            objIx = 0;
        }

        targetFile = fileNames[objIx+1];
        targetObj = getTargetObjMesh();
    }
}