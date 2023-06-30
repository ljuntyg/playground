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
    Text::Text(const std::wstring& text, float scale, std::shared_ptr<TextManager> textManager)
        : text(text), scale(scale), textManager(textManager)
    {
        calculateVertices();

        textManager->texts.emplace_back(this);
    }

    Text::~Text() {}

    void Text::calculateVertices()
    {
        GLfloat x = 0, y = 0; // Initialize x and y to the starting position of the text
        GLfloat baseline = 0;
        GLfloat lineHeight = textManager->idCharacterMap.at('A').height * 1.1f;

        characters.clear(); // Clear the previous characters

        for (wchar_t c : text)
        {
            if (c == '\n') 
            {
                // Handle newline
                x = 0; // Reset x
                y -= lineHeight * scale; // Move to next line
                continue;
            }

            if (textManager->idCharacterMap.find(c) == textManager->idCharacterMap.end()) 
            {
                std::wcout << "Character not found: " << c << '\n';
                continue;
            }
        
            Character ch = textManager->idCharacterMap.at(c);

            // Update font of Character to new selectedFont
            ch.font = this->textManager->selectedFont.value();

            GLfloat xpos = x + ch.xOffset * scale;
            GLfloat ypos = y + (baseline - ch.yOffset - ch.height) * scale;

            GLfloat w = ch.width * scale;
            GLfloat h = ch.height * scale;

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
            x += ch.xAdvance * scale;
        }
    }


    TextManager::TextManager()
    {
        loadFonts(FONTS_PATH);
        assert(!fonts.empty()); // Handle case with no fonts found
        selectedFont = fonts[7]; // DotGothic 16 for now
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
                std::vector<std::filesystem::path> textureFiles;

                for (const auto& fontEntry : std::filesystem::directory_iterator(entry))
                {
                    if (fontEntry.path().extension() == ".fnt")
                    {
                        fontFile = fontEntry.path();
                    }
                    else if (fontEntry.path().extension() == ".png")
                    {
                        textureFiles.emplace_back(fontEntry.path());
                    }
                }

                if (!fontFile.empty() && !textureFiles.empty())
                {
                    std::string fontName = entry.path().filename().string();
                    fonts.push_back(Font(fontFile, textureFiles, fontName));
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
        for (const auto& textureFile : font.textureFiles)
        {
            int width, height, nrChannels;
            unsigned char *data = stbi_load(textureFile.string().c_str(), &width, &height, &nrChannels, 0);
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

                font.textureIDs.push_back(textureID);

                // Set the texture dimensions in the font
                // Note: This assumes all textures have the same dimensions
                font.textureWidth = width;
                font.textureHeight = height;
            }
            else
            {
                std::cerr << "Failed to load texture file: " << textureFile << '\n';
                return;
            }
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

                idCharacterMap.emplace(charMap["id"], Character(font, charMap["id"], charMap["x"], charMap["y"], charMap["width"], charMap["height"], charMap["xoffset"], charMap["yoffset"], charMap["xadvance"], charMap["page"]));
            }
        }
    }

    void TextManager::nextFont()
    {
        int objIx = std::find(fonts.begin(), fonts.end(), selectedFont.value()) - fonts.begin();
        assert(objIx != fonts.size()); // Means file name not found

        if (objIx == fonts.size() - 1)
        {
            objIx = 0;
        }

        selectedFont = fonts[objIx+1];
        parseFont(selectedFont.value());
    }
}