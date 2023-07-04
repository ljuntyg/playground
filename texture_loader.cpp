#include <glad/glad.h>
#include <filesystem>
#include <iostream>

#include "texture_loader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace texturel
{
    GLuint loadFontTexture(const std::filesystem::path& texturePath, bool flipVertically, int nbrChannels)
    {
        stbi_set_flip_vertically_on_load(flipVertically); // Or the textures load upside down

        int width, height;
        std::cout << "Loading texture from: " << texturePath.string().c_str() << std::endl;
        unsigned char* texture = stbi_load(texturePath.string().c_str(), &width, &height, nullptr, nbrChannels);
        if (texture == NULL)
        {
            std::cerr << "Failed to load texture at: " << texturePath.string().c_str() << std::endl;
            return 0;
        }

        // Load the texture into OpenGL and store the texture ID
        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        GLenum format = (nbrChannels == 3) ? GL_RGB : GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, texture);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(texture);

        stbi_set_flip_vertically_on_load(false);

        return textureID;
    }

    bool loadCubemapTextures(GLuint textureID, std::filesystem::path facesCubemap[6]) 
    {
        for (unsigned int i = 0; i < 6; i++)
        {
            int width, height, nrChannels;
            unsigned char* data = stbi_load(facesCubemap[i].string().c_str(), &width, &height, &nrChannels, 3);
            if (data)
            {
                stbi_set_flip_vertically_on_load(false);
                glTexImage2D
                (
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                    0,
                    GL_RGB,
                    width,
                    height,
                    0,
                    GL_RGB,
                    GL_UNSIGNED_BYTE,
                    data
                );
                stbi_image_free(data);
            }
            else
            {
                std::cout << "Failed to load cubemap texture: " << facesCubemap[i] << std::endl;
                stbi_image_free(data);
                return false; // Fail the function as soon as a texture fails to load
            }
        }

        return true;
    }
}