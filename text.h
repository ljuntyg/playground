#pragma once

#include <filesystem>
#include <unordered_map>

namespace text
{
    struct Character;
    struct Line;
    class Font;

    std::vector<Character*> createText(std::wstring text, Font* font);
    std::vector<Line*> createLines(std::vector<Character*> characters, float* totalWidth, float* totalHeight);

    class Font
    {
    public:
        Font(std::string fontName, std::filesystem::path fontFolderPath);
        ~Font();

        void registerCharacter(Character* character);
        void removeCharacter(Character* character);
        Character* getCharacter(const wchar_t &wc, bool* result);

        static Font* getDefaultFont();
        static std::filesystem::path defaultFontPath;
        static Font* defaultFont;
        
        std::string getFontName();
        std::vector<std::filesystem::path> getPngPaths();
        std::unordered_map<int, Character*> getIdCharacterMap();
 
        float getSize();
        float getTextureWidth();
        float getTextureHeight();
        float getLineHeight();
        float getAscender();
        float getDescender();

    private:
        std::string fontName;
        std::filesystem::path fontFolderPath;
        std::filesystem::path jsonPath;
        std::vector<std::filesystem::path> pngPaths; // E.g. res/fonts/Bungee_Inline

        std::vector<Character*> characters;
        std::unordered_map<int, Character*> idCharacterMap;

        float size;
        float textureWidth;
        float textureHeight;
        float emSize;
        float lineHeight;
        float ascender;
        float descender;
        float underlineY;
        float underlineThickness;

        bool loadFontPaths(std::filesystem::path fontFolderPath);
        bool bindCharacterIDs();
    };

    struct Character
    {
        Font* font;
        unsigned int id;
        float advance;
        float planeLeft; // Plane variables are in EMs
        float planeBottom;
        float planeRight;
        float planeTop;
        float atlasLeft; // Atlas variables are in pixels
        float atlasBottom;
        float atlasRight;
        float atlasTop;

        Character(Font* font, unsigned int id, float advance, 
                float planeLeft, float planeBottom, float planeRight, float planeTop,
                float atlasLeft, float atlasBottom, float atlasRight, float atlasTop)
            : font(font), id(id), advance(advance),
            planeLeft(planeLeft), planeBottom(planeBottom), planeRight(planeRight), planeTop(planeTop),
            atlasLeft(atlasLeft), atlasBottom(atlasBottom), atlasRight(atlasRight), atlasTop(atlasTop)
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
        float yPosition; // yPosition reaches to top of Line, not bottom
        float height;
        float maxAscender;

        Line() : startX(0), endX(0), yPosition(0), height(0) {}
    };
}