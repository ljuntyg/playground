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

        std::unordered_map<int, Character> getIdCharacterMap();
        std::vector<unsigned char*> getTextures();

        int getTextureNbrChannels();
        float getTextureWidth();
        float getTextureHeight();

    private:
        friend struct Character;
        std::vector<Character*> characters;

        std::string fontName;
        std::filesystem::path fontFolderPath;
        std::filesystem::path fntPath;
        std::vector<std::filesystem::path> pngPaths; // E.g. res/fonts/Bungee_Inline

        std::unordered_map<int, Character> idCharacterMap;
        std::vector<unsigned char*> textures;
        int textureNbrChannels;
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

        Character(Font* font, int id, int x, int y, int width, int height, int xOffset, int yOffset, int xAdvance, int page) 
            : font(font), id(id), x(x), y(y), width(width), height(height), xOffset(xOffset), yOffset(yOffset), xAdvance(xAdvance), page(page) 
        {
            if (font == nullptr) 
            {
                std::cerr << "Null font used to create character" << std::endl;
            }

            font->registerCharacter(this);
        }

        ~Character()
        {
            if (font)
            {
                font->removeCharacter(this);
            }
        }
    };

    struct Line {
        std::vector<text::Character*> characters;
        float startX;
        float endX;
        float yPosition;
        float height;

        Line() : startX(0), endX(0), yPosition(0), height(0) {}
    };
}