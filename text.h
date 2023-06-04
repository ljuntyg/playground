#pragma once

#include <filesystem>
#include <unordered_map>

namespace text
{
    struct Character;
    class Font;

    std::vector<Character*> createText(std::wstring text, Font* font);

    class Font
    {
    public:
        Font(std::string fontName, std::filesystem::path fontFolderPath);
        ~Font();

        void registerCharacter(Character* character);
        void removeCharacter(Character* character);
        Character* getCharacter(const wchar_t &wc, bool* result);

        static std::filesystem::path defaultFontPath;
        static Font defaultFont;
        static Font& getDefaultFont();

    private:
        friend struct Character;
        std::vector<Character*> characters;

        std::string fontName;
        std::filesystem::path fontFolderPath;
        std::filesystem::path fntPath;
        std::vector<std::filesystem::path> pngPaths; // E.g. res/fonts/Bungee_Inline

        std::unordered_map<int, Character> idCharacterMap;
        std::vector<unsigned char*> textures;
        float textureWidth;
        float textureHeight;

        bool loadFontPaths(std::filesystem::path fontFolderPath);
        bool loadTextures();
        bool bindCharacterIds();
    };

    struct Character
    {
        Font* font;
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

        Character(Font* font, int id, int x, int y, int width, int height, int xOffset, int yOffset, int xAdvance, int page) 
            : font(font), id(id), x(x), y(y), width(width), height(height), xOffset(xOffset), yOffset(yOffset), xAdvance(xAdvance), page(page) 
        {
            if (font == nullptr) 
            {
                std::cerr << "Null font used to create character" << std::endl;
            }
            font->registerCharacter(this);

            // Texture coordinates
            float texX = x / font->textureWidth;
            float texY = y / font->textureHeight;
            float texWidth = width / font->textureWidth;
            float texHeight = height / font->textureHeight;

            textureCoords = {
                texX, texY, // Top left
                texX, texY + texHeight, // Bottom left
                texX + texWidth, texY + texHeight, // Bottom right
                texX, texY, // Top left
                texX + texWidth, texY + texHeight, // Bottom right
                texX + texWidth, texY // Top right
            };
        }

        ~Character()
        {
            if (font)
            {
                font->removeCharacter(this);
            }
        }
    };
}