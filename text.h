#pragma once

#include <vector>
#include <filesystem>
#include <iostream>
#include <unordered_map>
#include <memory>
#include <optional>

namespace text
{
    const std::string FONTS_PATH = "res/fonts";

    // Forward declarations
    struct Character;
    class TextManager;

    struct Font
    {
        std::filesystem::path fontFile;
        std::filesystem::path textureFile;
        std::string name;

        Font(const std::filesystem::path& fontFile, const std::filesystem::path& textureFile, const std::string& name)
            : fontFile(fontFile), textureFile(textureFile), name(name) {}

        bool operator==(const Font& other) const 
        {
            return fontFile == other.fontFile && textureFile == other.textureFile && name == other.name;
        }
    };

    struct Character
    {
        int id;
        int x;
        int y;
        int width;
        int height;
        int xOffset;
        int yOffset;
        int xAdvance;

        Character(int id, int x, int y, int width, int height, int xOffset, int yOffset, int xAdvance) 
            : id(id), x(x), y(y), width(width), height(height), xOffset(xOffset), yOffset(yOffset), xAdvance(xAdvance) {}
    };

    class Text
    {
    public:
        Text(const std::string& text, std::shared_ptr<TextManager> textManager); // Convert text to Character vector in constructor
        ~Text();
    private:
        std::shared_ptr<TextManager> textManager;
        std::string text;
        std::vector<Character> characters;
    };

    class TextManager
    {
    public:
        TextManager();
        ~TextManager();

        void loadFonts(const std::string& fontPath);
        void parseFont(const Font& font);
    private:
        friend class Text;

        std::vector<Font> fonts;
        std::vector<Text> texts;

        std::optional<Font> selectedFont; // If this is changed, then idCharacterMap needs to be updated for the new font, use parseFont
        std::unordered_map<int, Character> idCharacterMap;
    };
}