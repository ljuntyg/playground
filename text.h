#pragma once

#include <filesystem>

namespace text
{
    class Font
    {
    public:
        Font(std::string fontName, std::string fontPath);
        ~Font();

    private:
        friend struct Character;

        std::string fontName;
        std::string fontPath;

        std::filesystem::path fntPath;
        std::vector<std::filesystem::path> pngPaths; // E.g. res/fonts/Bungee_Inline
        std::vector<unsigned char*> textures;

        float textureWidth;
        float textureHeight;

        bool loadFontPaths(std::string fontPath);
        bool loadTextures();
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

        std::vector<float> vertices;
        std::vector<float> textureCoords;

        Character(Font font, int id, int x, int y, int width, int height, int xOffset, int yOffset, int xAdvance, int page) 
            : font(font), id(id), x(x), y(y), width(width), height(height), xOffset(xOffset), yOffset(yOffset), xAdvance(xAdvance), page(page) 
        {
            // Texture coordinates
            float texX = x / font.textureWidth;
            float texY = y / font.textureHeight;
            float texWidth = width / font.textureWidth;
            float texHeight = height / font.textureHeight;

            textureCoords = {
                texX, texY, // Top left
                texX, texY + texHeight, // Bottom left
                texX + texWidth, texY + texHeight, // Bottom right
                texX, texY, // Top left
                texX + texWidth, texY + texHeight, // Bottom right
                texX + texWidth, texY // Top right
            };
        }
    };
}