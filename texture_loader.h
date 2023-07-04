#pragma once

#include <array>

namespace texturel
{
    GLuint loadFontTexture(const std::filesystem::path& texturePath, bool flipVertically = true, int nbrChannels = 3);
    bool loadCubemapTextures(GLuint textureID, std::filesystem::path facesCubemap[6]); // TODO: Implement this
}