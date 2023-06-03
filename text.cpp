#include <iostream>

#include "text.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace text
{
    Font::Font(std::string fontName, std::string fontPath)
        : fontName(fontName), fontPath(fontPath)
    {
        if (!loadFontPaths(fontPath))
        {
            std::cerr << "Unable to load path to fonts, unable to proceed to load textures" << std::endl;
            return;
        }

        if (!loadTextures())
        {
            std::cerr << "Unable to load textures" << std::endl;
        }
    }

    Font::~Font() 
    {
        for (auto& texture : textures)
        {
            if (texture != nullptr)
            {
                stbi_image_free(texture);
                texture = nullptr;
            }
        }
    }

    bool Font::loadFontPaths(std::string fontPath)
    {
        std::filesystem::path searchPath(fontPath);
        std::cout << "Checking existence of: " << std::filesystem::absolute(searchPath) << std::endl;

        if (!std::filesystem::exists(searchPath))
        {
            searchPath = std::filesystem::current_path().parent_path() / fontPath;

            if (std::filesystem::exists(searchPath))
            {
                std::filesystem::current_path(std::filesystem::current_path().parent_path());

                std::cout << "Changed working directory to: "
                        << std::filesystem::current_path() << std::endl;
            }
            else
            {
                return false;
            }
        }

        // Font folder found, load the font paths
        try
        {
            for (const auto& entry : std::filesystem::directory_iterator(searchPath))
            {
                if (entry.path().extension() == ".fnt")
                {
                    fntPath = entry.path();
                }
                else if (entry.path().extension() == ".png")
                {
                    pngPaths.emplace_back(entry.path());
                }
            }
        }
        catch (const std::filesystem::filesystem_error& e) 
        {
            std::cerr << "Error: " << e.what() << std::endl;
        }

        if (fntPath.empty() || pngPaths.empty())
        {
            return false;
        }

        return true;
    }

    bool Font::loadTextures()
    {
        for (auto& path : pngPaths)
        {
            int width, height, channels;
            unsigned char* texture = stbi_load(path.string().c_str(), &width, &height, &channels, 0);
            if (texture == NULL)
            {
                return false;
            }

            textures.emplace_back(texture);
        }

        return true;
    }
}