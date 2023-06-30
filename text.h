#pragma once

#include <vector>
#include <filesystem>
#include <iostream>
#include <unordered_map>
#include <memory>
#include <optional>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <glm/glm.hpp>

namespace text
{
    const std::string FONTS_PATH = "res/fonts";

    // Forward declarations
    struct Character;
    class TextManager;

    struct Font
    {
        std::filesystem::path fontFile;
        std::vector<std::filesystem::path> textureFiles;
        
        float textureWidth;
        float textureHeight;

        std::vector<GLuint> textureIDs; // Vector in case a font requires multiple textures
        std::string name;

        Font(const std::filesystem::path& fontFile, const std::vector<std::filesystem::path>& textureFiles, const std::string& name) // Destructor?
            : fontFile(fontFile), textureFiles(textureFiles), name(name) {}

        bool operator==(const Font& other) const 
        {
            return fontFile == other.fontFile && textureFiles == other.textureFiles && name == other.name;
        }
    };

    struct Character
    {
        Font font;
        int id;
        int x;
        int y;
        int width;
        int height;
        int xOffset;
        int yOffset;
        int xAdvance;
        int page;

        std::vector<GLfloat> vertices;
        std::vector<GLfloat> texCoords;

        Character(Font font, int id, int x, int y, int width, int height, int xOffset, int yOffset, int xAdvance, int page) 
            : font(font), id(id), x(x), y(y), width(width), height(height), xOffset(xOffset), yOffset(yOffset), xAdvance(xAdvance), page(page) 
        {
            // Texture coordinates
            float texX = x / static_cast<float>(font.textureWidth);
            float texY = y / static_cast<float>(font.textureHeight);
            float texWidth = width / static_cast<float>(font.textureWidth);
            float texHeight = height / static_cast<float>(font.textureHeight);

            texCoords = {
                texX, texY, // Top left
                texX, texY + texHeight, // Bottom left
                texX + texWidth, texY + texHeight, // Bottom right
                texX, texY, // Top left
                texX + texWidth, texY + texHeight, // Bottom right
                texX + texWidth, texY // Top right
            };
        }
    };

    class Text
    {
    public:
        Text(const std::wstring& text, float scale, std::shared_ptr<TextManager> textManager); // Convert text to Character vector in constructor, wstring for Chinese characters etc.
        ~Text();

        void calculateVertices();

        std::wstring text;
        float scale;
        std::shared_ptr<TextManager> textManager;
        std::vector<Character> characters;
    };

    class TextManager
    {
    public:
        TextManager();
        ~TextManager();

        void loadFonts(const std::string& fontPath);
        void parseFont(Font& font);
        void nextFont();

        std::vector<std::shared_ptr<Text>> texts;
    private:
        friend class Text;

        std::vector<Font> fonts;
        std::optional<Font> selectedFont; // If this is changed, then idCharacterMap needs to be updated for the new font, use parseFont
        std::unordered_map<int, Character> idCharacterMap;
    };
}