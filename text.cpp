#include <cassert>
#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>

#include "text.h"

namespace text
{
    Text::Text(const std::string& text, std::shared_ptr<TextManager> textManager) : text(text), textManager(textManager)
    {
        textManager->texts.emplace_back(*this);

        // Fill out characters vector with corresponding Character to ones in text
        for (const char& c : text)
        {
            characters.emplace_back(textManager->idCharacterMap.at(c));
        }

        assert(text.size() == characters.size());
    }

    Text::~Text() {}

    TextManager::TextManager()
    {
        loadFonts(FONTS_PATH);
        assert(!fonts.empty()); // Handle case with no fonts found
        selectedFont = fonts[0];
        parseFont(*selectedFont);
    }

    TextManager::~TextManager() {}

    void TextManager::loadFonts(const std::string& fontPath)
    {
        for (const auto& entry : std::filesystem::directory_iterator(fontPath))
        {
            if (entry.is_directory())
            {
                std::filesystem::path fontFile;
                std::filesystem::path textureFile;

                for (const auto& fontEntry : std::filesystem::directory_iterator(entry))
                {
                    if (fontEntry.path().extension() == ".fnt")
                    {
                        fontFile = fontEntry.path();
                    }
                    else if (fontEntry.path().extension() == ".png")
                    {
                        textureFile = fontEntry.path();
                    }
                }

                if (!fontFile.empty() && !textureFile.empty())
                {
                    std::string fontName = entry.path().filename().string();
                    fonts.push_back(Font(fontFile, textureFile, fontName));
                }
            }
        }
    }


    void TextManager::parseFont(const Font& font)
    {
        // Assert that font exists in fonts
        assert(std::find(fonts.begin(), fonts.end(), font) != fonts.end());

        // Clear the map since any parsed Character will be placed at back, problematic if selectedFont has changed
        idCharacterMap.clear();

        std::ifstream fontFile(font.fontFile);
        if (!fontFile)
        {
            std::cerr << "Failed to open font file: " << font.fontFile << '\n';
            return;
        }

        std::string line;
        while (std::getline(fontFile, line))
        {
            if (line.substr(0, 8) == "char id=")
            {
                std::istringstream iss(line);
                std::string key;
                std::map<std::string, int> charMap;

                // Assuming each line is in the format "key=value"
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

                idCharacterMap.emplace(charMap["id"], Character(charMap["id"], charMap["x"], charMap["y"], charMap["width"], charMap["height"], charMap["xoffset"], charMap["yoffset"], charMap["xadvance"]));
            }
        }
    }
}