#include <cassert>
#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>

#include "text.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace text
{
    Text::Text(const std::string& text, std::shared_ptr<TextManager> textManager) : text(text), textManager(textManager)
    {
        GLfloat x = 0, y = 0; // Initialize x and y to the starting position of the text

        for (char c : text)
        {
            Character ch = textManager->idCharacterMap.at(c);

            GLfloat xpos = x + ch.xOffset;
            GLfloat ypos = y - ch.yOffset;

            GLfloat w = ch.width;
            GLfloat h = ch.height;

            ch.vertices = {
                xpos,     ypos + h, // Bottom left
                xpos,     ypos,     // Top left
                xpos + w, ypos,     // Top right
                xpos,     ypos + h, // Bottom left
                xpos + w, ypos,     // Top right
                xpos + w, ypos + h  // Bottom right
            };

            // Store the character with updated vertices
            characters.emplace_back(ch);

            // Advance cursor for next glyph
            x += ch.xAdvance;
        }
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

    void TextManager::parseFont(Font& font)
    {
        // Assert that font exists in fonts
        assert(std::find(fonts.begin(), fonts.end(), font) != fonts.end());

        // Clear the map since any parsed Character will be placed at back, problematic if selectedFont has changed
        idCharacterMap.clear();

        // Load the font texture
        int width, height, nrChannels;
        unsigned char *data = stbi_load(font.textureFile.string().c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            GLuint textureID;
            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);

            // set the texture wrapping/filtering options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            // load and generate the texture
            if(nrChannels == 3)
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            else if(nrChannels == 4)
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            stbi_image_free(data);

            font.textureID = textureID; // Assign the textureID to the font

            // Set the texture dimensions in the font
            font.textureWidth = width;
            font.textureHeight = height;
        }
        else
        {
            std::cerr << "Failed to load texture file: " << font.textureFile << '\n';
            return;
        }

        std::ifstream fontFile(font.fontFile);
        if (!fontFile)
        {
            std::cerr << "Failed to open font file: " << font.fontFile << '\n';
            return;
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

                idCharacterMap.emplace(charMap["id"], Character(font, charMap["id"], charMap["x"], charMap["y"], charMap["width"], charMap["height"], charMap["xoffset"], charMap["yoffset"], charMap["xadvance"]));
            }
        }
    }
}