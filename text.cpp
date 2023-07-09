#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

#include "json.h"
#include "text.h"

namespace text
{
    // TODO: Texts created with this (actually, everywhere) aren't being registered to Font's characters (?)
    std::vector<Character*> createText(std::wstring text, Font* font)
    {
        std::vector<Character*> characters;
        for (const auto& wc : text)
        {
            bool charFound = false;
            Character* character = font->getCharacter(wc, &charFound);
            if (charFound == false)
            {
                std::wcerr << "Character not found in ID-character map, wide char: " << wc << std::endl;
            }

            characters.emplace_back(character);
        }

        return characters;
    }

    std::vector<Line*> createLines(std::vector<Character*> characters, float* totalWidth, float* totalHeight)
    {
        std::vector<text::Line*> lines;
        lines.emplace_back(new text::Line());
        if (characters.empty()) {
            return lines;
        }

        float fontSize = characters[0]->font->getSize();
        float lineWidth = 0, lineHeight = 0, maxAscender = 0, yPos = 0;
        for (const auto ch : characters)
        {
            if (ch->id == '\n')
            {
                lines.back()->height = lineHeight;
                lines.back()->maxAscender = maxAscender; // Set maxAscender for the line
                lines.back()->endX = lineWidth;
                *totalWidth = std::max(*totalWidth, lineWidth); // Keep the maximum line width
                *totalHeight += lineHeight;

                yPos -= lineHeight; // Update the yPosition for the next line

                lines.emplace_back(new text::Line());
                lines.back()->startX = 0;
                lines.back()->yPosition = yPos;

                lineWidth = 0;
                lineHeight = 0;
                maxAscender = 0; // Reset maxAscender for the next line
                continue;
            }

            lines.back()->characters.emplace_back(ch);
            lineWidth += ch->advance * fontSize;

            // Different Characters can have different Fonts,
            // so find the Font with the max lineHeight and maxAscender
            float characterHeight = ch->font->getLineHeight() * ch->font->getSize(); 
            lineHeight = std::max(lineHeight, characterHeight);

            float characterAscender = ch->font->getAscender() * ch->font->getSize();
            maxAscender = std::max(maxAscender, characterAscender);
        }

        // Update height, maxAscender, totalWidth, totalHeight, and endX for last line
        lines.back()->height = lineHeight;
        lines.back()->maxAscender = maxAscender;
        lines.back()->endX = lineWidth;
        *totalWidth = std::max(*totalWidth, lineWidth);
        *totalHeight += lineHeight;

        return lines;
    }

    // TODO: If the requested font folder exists but does not contain the files necessary to load the font, then this constructor
    // will return an incomplete Font which will cause issues elsewhere, in particular when calling createText using the Font
    Font::Font(std::string fontName, std::filesystem::path fontFolderPath)
        : fontName(fontName), fontFolderPath(fontFolderPath)
    {
        if (!loadFontPaths(fontFolderPath))
        {
            std::cerr << "Unable to load path to fonts, unable to proceed to load textures, returning default font" << std::endl;
            return;
        }

        if (!bindCharacterIDs())
        {
            std::cerr << "Failed to bind IDs for characters" << std::endl;
        }
    }

    Font::~Font() 
    {
        for (auto* character : characters) 
        {
            if (character && this != getDefaultFont())
            {
                character->font = getDefaultFont(); // Replace destroyed Font with defaultFont
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
            return idCharacterMap.at(63); // Codepoint decimal 63 is question mark
        }

        *result = true;
        return idCharacterMap.at(wc);
    }

    std::filesystem::path text::Font::defaultFontPath;
    Font* Font::defaultFont = nullptr;

    Font* Font::getDefaultFont() 
    {
        if (defaultFont == nullptr) // Only initialize if it hasn't been initialized already
        {
            defaultFont = new Font(defaultFontPath.filename().string(), defaultFontPath);
        }

        return defaultFont;
    }

    std::string Font::getFontName()
    {
        return fontName;
    }

    std::vector<std::filesystem::path> Font::getPngPaths()
    {
        return pngPaths;
    }

    std::unordered_map<int, Character*> Font::getIdCharacterMap()
    {
        return idCharacterMap;
    }

    float Font::getSize()
    {
        return size;
    }

    float Font::getTextureWidth()
    {
        return textureWidth;
    }

    float Font::getTextureHeight()
    {
        return textureHeight;
    }
    
    float Font::getLineHeight()
    {
        return lineHeight;
    }

    float Font::getAscender()
    {
        return ascender;
    }

    float Font::getDescender()
    {
        return descender;
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

                std::cout << "Changed working directory to: " << std::filesystem::current_path() << std::endl;
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
                auto extension = entry.path().extension();
                if (entry.path().extension() == ".json")
                {
                    jsonPath = entry.path();
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

        if (jsonPath.empty() || pngPaths.empty())
        {
            return false;
        }

        return true;
    }

    bool Font::bindCharacterIDs() 
    {
        // Open the file
        std::ifstream jsonFile(jsonPath);

        // Parse the file into a JSON object
        nlohmann::json j;
        jsonFile >> j;

        // Parse the metrics
        emSize = j["metrics"]["emSize"];
        lineHeight = j["metrics"]["lineHeight"];
        ascender = j["metrics"]["ascender"];
        descender = j["metrics"]["descender"];
        underlineY = j["metrics"]["underlineY"];
        underlineThickness = j["metrics"]["underlineThickness"];

        // Parse the size and texture size
        size = j["atlas"]["size"];
        textureWidth = j["atlas"]["width"];
        textureHeight = j["atlas"]["height"];

        // Parse the glyphs
        for (auto& glyph : j["glyphs"])
        {
            unsigned int id = glyph["unicode"];
            float advance = glyph["advance"];

            float planeLeft = 0.0f;
            float planeBottom = 0.0f;
            float planeRight = 0.0f;
            float planeTop = 0.0f;
            if (glyph.contains("planeBounds")) {
                planeLeft = glyph["planeBounds"]["left"];
                planeBottom = glyph["planeBounds"]["bottom"];
                planeRight = glyph["planeBounds"]["right"];
                planeTop = glyph["planeBounds"]["top"];
            }

            float atlasLeft = 0.0f;
            float atlasBottom = 0.0f;
            float atlasRight = 0.0f;
            float atlasTop = 0.0f;
            if (glyph.contains("atlasBounds")) {
                atlasLeft = glyph["atlasBounds"]["left"];
                atlasBottom = glyph["atlasBounds"]["bottom"];
                atlasRight = glyph["atlasBounds"]["right"];
                atlasTop = glyph["atlasBounds"]["top"];
            }

            idCharacterMap[id] = new Character(this, id, advance, planeLeft, planeBottom, planeRight, planeTop, atlasLeft, atlasBottom, atlasRight, atlasTop);
        }

        // Add Character to represent new-line with id of 10
        if (!idCharacterMap.empty())
        {
            idCharacterMap[10] = new Character(this, (unsigned int)10, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f);

            // Add a space character if it does not exist in the JSON file
            if (idCharacterMap.find(32) == idCharacterMap.end()) // Check if space character exists
            {
                // Space character does not exist, create one, codepoint 97 for "a"
                idCharacterMap[32] = new Character(this, (unsigned int)32, idCharacterMap.at(97)->advance, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
            }
        }

        jsonFile.close();

        return true;
    }
}