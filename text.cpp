#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

#include "text.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace text
{
    std::vector<Character*> createText(std::wstring text, Font* font)
    {
        std::vector<Character*> characters;
        for (const auto& wc : text)
        {
            bool charFound = false;
            Character* character = font->getCharacter(wc, &charFound);
            if (charFound == false)
            {
                std::cerr << "Character not found in ID-character map" << std::endl;
            }

            characters.emplace_back(character);
        }

        return characters;
    }

    Font::Font(std::string fontName, std::filesystem::path fontFolderPath)
        : fontName(fontName), fontFolderPath(fontFolderPath)
    {
        if (!loadFontPaths(fontFolderPath))
        {
            std::cerr << "Unable to load path to fonts, unable to proceed to load textures" << std::endl;
            return;
        }

        if (!loadTextures())
        {
            std::cerr << "Unable to load textures" << std::endl;
        }

        if (!bindCharacterIds())
        {
            std::cerr << "Failed to bind IDs for characters" << std::endl;
        }
    }

    Font::~Font() 
    {
        for (auto& texture : textures)
        {
            if (texture)
            {
                stbi_image_free(texture);
                texture = nullptr;
            }
        }

        for (auto* character : characters)
        {
            if (character)
            {
                character->font = &getDefaultFont(); // Replace destroyed Font with defaultFont
                character->font->registerCharacter(character);
            }
        }
    }

    void Font::registerCharacter(Character* character) 
    {
        characters.push_back(character);
    }

    void Font::removeCharacter(Character* character) 
    {
        characters.erase(std::remove(characters.begin(), characters.end(), character), characters.end());
    }

    Character* Font::getCharacter(const wchar_t &wc, bool* result)
    {
        if (idCharacterMap.find(wc) == idCharacterMap.end()) 
        {
            std::cerr << "Character not found, ID: " << (int)wc << std::endl;
            *result = false;
            return &idCharacterMap.at(63); // 63 is question mark, hopefully lol
        }

        *result = true;
        return &idCharacterMap.at(wc);
    }

    std::filesystem::path text::Font::defaultFontPath;

    Font& Font::getDefaultFont() 
    {
        static Font defaultFont(defaultFontPath.string(), defaultFontPath);
        return defaultFont;
    }

    std::unordered_map<int, Character> Font::getIdCharacterMap()
    {
        return idCharacterMap;
    }

    std::vector<unsigned char*> Font::getTextures()
    {
        return textures;
    }

    int Font::getTextureNbrChannels()
    {
        return textureNbrChannels;
    }

    float Font::getTextureWidth()
    {
        return textureWidth;
    }

    float Font::getTextureHeight()
    {
        return textureHeight;
    }

    bool Font::loadFontPaths(std::filesystem::path fontFolderPath)
    {
        std::filesystem::path searchPath(fontFolderPath);
        std::cout << "Checking existence of: " << std::filesystem::absolute(searchPath) << std::endl;

        if (!std::filesystem::exists(searchPath))
        {
            searchPath = std::filesystem::current_path().parent_path() / fontFolderPath;

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

        if (defaultFontPath.empty())
        {
            try
            {
                std::filesystem::path parentPath = std::filesystem::absolute(fontFolderPath).parent_path();
                // Find new defaultFontPath for default Font in font parent path (fonts folder)
                for (const auto& entry : std::filesystem::directory_iterator(parentPath)) 
                {
                    if (entry.is_directory()) 
                    {
                        defaultFontPath = entry;
                        std::cout << "Default font path: " << defaultFontPath << std::endl;
                        break;
                    }
                }
            }
            catch (const std::filesystem::filesystem_error& e)
            {
                std::cerr << "Error setting default path: " << e.what() << std::endl;
                return false;
            }
        }

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
            std::cerr << "Error reading font path files: " << e.what() << std::endl;
            return false;
        }

        if (fntPath.empty() || pngPaths.empty())
        {
            return false;
        }

        return true;
    }

    bool Font::loadTextures()
    {
        int width, height, channels;
        for (auto& path : pngPaths)
        {
            std::cout << "Loading texture from: " << path.string().c_str() << std::endl;
            unsigned char* texture = stbi_load(path.string().c_str(), &width, &height, &channels, 0);
            if (texture == NULL)
            {
                return false;
            }

            textures.emplace_back(texture);
        }

        textureNbrChannels = channels;
        textureWidth = (float)width;
        textureHeight = (float)height;
        
        return true;
    }

    bool Font::bindCharacterIds()
    {
        std::ifstream fontFile(fntPath);
        if (!fontFile)
        {
            std::cerr << "Failed to open font file: " << fntPath << '\n';
            return false;
        }

        // Create id-Character map
        std::string line;
        while (std::getline(fontFile, line))
        {
            if (line.substr(0, 8) == "char id=")
            {
                std::istringstream iss(line);
                std::string key;
                std::map<std::string, int> charMap;

                while (iss >> key)
                {
                    size_t pos = key.find('=');
                    if (pos != std::string::npos)
                    {
                        std::string actualKey = key.substr(0, pos);
                        int value = std::stoi(key.substr(pos+1));
                        charMap[actualKey] = value;
                    }
                }

                idCharacterMap.emplace(charMap["id"], Character(this, charMap["id"], charMap["x"], charMap["y"], charMap["width"], charMap["height"], charMap["xoffset"], charMap["yoffset"], charMap["xadvance"], charMap["page"]));
            }
        }

        return true;
    }
}